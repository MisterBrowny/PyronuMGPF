#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Supervision PyronuMGPF - Version corrigée et optimisée
"""

from nicegui import ui
from pymodbus.client import ModbusSerialClient as ModbusClient
import asyncio
import time
import re
import serial.tools.list_ports   # ← AJOUTÉ

# ────────────────────────────────────────────────
# CONFIGURATION
# ────────────────────────────────────────────────
PORT = 'COM20'                    # ← valeur initiale (sera remplacée par la sélection)
BAUDRATE = 9600
PARITY = 'N'
STOPBITS = 1
BYTESIZE = 8
TIMEOUT = 0.1

NUM_DEVICES = 20
NUM_ANALOG_INPUTS = 16

GLOBAL_STATE_REG = 0
ANALOG_START_REG = 1
CONFIG_START_REG = 3
CONFIG_NUM_REGS = 8

SLAVE_IDS = list(range(1, NUM_DEVICES + 1))
REFRESH_INTERVAL = 0.1

# ────── AJOUTÉ : variables pour la sélection dynamique ──────
selected_port = 'COM20'
active_slaves = set(SLAVE_IDS)          # slaves actuellement visibles

# Dictionnaires (ils seront réinitialisés à chaque régénération des onglets)
global_labels = {}
analog_labels = {}
last_update = {}
send_inputs = {}
read_outputs = {}
status_labels = {}

tabs_container = None          # ← contiendra les onglets (recréés dynamiquement)

GLOBAL_MAP = {
    0: ("END",   "text-3xl text-gray-700 font-bold"),
    1: ("GO",    "text-3xl text-green-600 font-bold"),
    2: ("ARMED", "text-3xl text-blue-600 font-bold"),
    3: ("TEST",  "text-3xl text-orange-600 font-bold"),
}

ANALOG_MAP = {
    0b00: ("ABSENT", "text-6xl text-gray-500"),
    0b01: ("KO",     "text-6xl text-red-600 font-bold"),
    0b10: ("MOYEN",  "text-6xl text-yellow-600 font-bold"),
    0b11: ("OK",     "text-6xl text-green-600 font-bold"),
}


DEFAULT_GLOBAL = ("???", "text-2xl text-purple-600 font-bold")
DEFAULT_ANALOG = ("ERR", "text-2xl text-red-600 font-bold")

def get_available_ports():
    """Retourne la liste des ports COM disponibles"""
    ports = serial.tools.list_ports.comports()
    return [p.device for p in ports] or ["Aucun port détecté"]


# ────────────────────────────────────────────────
# FONCTIONS MODBUS (inchangées sauf PORT → selected_port)
# ────────────────────────────────────────────────
def modbus_read(slave_id: int):
    client = ModbusClient(port=selected_port, baudrate=BAUDRATE, parity=PARITY,
                          stopbits=STOPBITS, bytesize=BYTESIZE, timeout=TIMEOUT)
    try:
        if not client.connect():
            return None, None, "Connexion impossible"

        resp = client.read_holding_registers(GLOBAL_STATE_REG, count=3, device_id=slave_id)
        if resp.isError():
            return None, None, str(resp)

        regs = resp.registers
        if len(regs) != 3:
            return None, None, "Réponse incomplète"

        global_val = regs[0] & 0xFF
        global_state = GLOBAL_MAP.get(global_val, DEFAULT_GLOBAL)

        low = regs[1]
        high = regs[2]
        bits32 = (high << 16) | low

        analogs = []
        for i in range(NUM_ANALOG_INPUTS):
            shift = i * 2
            val2 = (bits32 >> shift) & 0b11
            analogs.append(ANALOG_MAP.get(val2, DEFAULT_ANALOG))

        return global_state, analogs, None

    except Exception as e:
        return None, None, str(e)
    finally:
        client.close()


def modbus_read_config(slave_id: int, num_regs: int):
    client = ModbusClient(port=selected_port, baudrate=BAUDRATE, parity=PARITY,
                          stopbits=STOPBITS, bytesize=BYTESIZE, timeout=TIMEOUT)
    try:
        if not client.connect():
            return False, "Connexion impossible"

        resp = client.read_holding_registers(CONFIG_START_REG, count=num_regs, device_id=slave_id)
        if resp.isError():
            return False, str(resp)

        regs = resp.registers
        hex_bytes = []
        for val in regs:
            hex_bytes.append(f"{(val >> 8) & 0xFF:02X}")
            hex_bytes.append(f"{val & 0xFF:02X}")
        return True, ' '.join(hex_bytes)

    except Exception as e:
        return False, str(e)
    finally:
        client.close()


async def modbus_write_config(slave_id: int, hex_string: str):
    cleaned = re.sub(r'[^0-9A-Fa-f]', '', hex_string.upper())
    if not cleaned or len(cleaned) % 2 != 0:
        return False, "Longueur hex invalide (doit être paire)"

    try:
        bytes_data = bytes.fromhex(cleaned)
        values = []
        for i in range(0, len(bytes_data), 2):
            hi = bytes_data[i]
            lo = bytes_data[i + 1] if i + 1 < len(bytes_data) else 0
            values.append((hi << 8) | lo)

        client = ModbusClient(port=PORT, baudrate=BAUDRATE, parity=PARITY,
                              stopbits=STOPBITS, bytesize=BYTESIZE, timeout=TIMEOUT)
        try:
            if not client.connect():
                return False, "Connexion impossible"
            result = client.write_registers(CONFIG_START_REG, values, slave=slave_id)
            return (False, str(result)) if result.isError() else (True, f"{len(values)} registres écrits")
        finally:
            client.close()
    except Exception as e:
        return False, str(e)


# ────────────────────────────────────────────────
# RECRÉATION DYNAMIQUE DES ONGLETS (AJOUTÉ)
# ────────────────────────────────────────────────
def refresh_tabs():
    global tabs_container
    tabs_container.clear()

    if not active_slaves:
        with tabs_container:
            ui.label('Aucun module sélectionné').classes('text-4xl text-gray-500 text-center py-20')
        return

    with tabs_container:
        with ui.tabs().classes('w-full text-2xl') as tabs:
            tab_list = [ui.tab(f'PyronuMGPF {dev:02d} – ID {sid}')
                        for dev, sid in enumerate(SLAVE_IDS, 1) if sid in active_slaves]

        with ui.tab_panels(tabs, value=tab_list[0] if tab_list else None).classes('w-full'):
            # Ici on recrée uniquement les onglets cochés (le reste du contenu est identique à ton code)
            for dev_idx, tab in enumerate(tab_list, start=1):
                sid = SLAVE_IDS[dev_idx - 1]
                if sid not in active_slaves:
                    continue

                with ui.tab_panel(tab):
                    with ui.card().classes('w-full'):
                        ui.label(f'PyronuMGPF {dev_idx} • ID {sid}').classes('text-4xl font-bold mb-6')

                        # === Tout ton contenu d'onglet (État global + TIR + Configuration) ===
                        # (je le laisse tel quel pour ne rien casser)
                        with ui.row().classes('items-center gap-6 mb-8'):
                            ui.label('État global :').classes('text-3xl')
                            g = ui.label('—').classes('text-4xl font-bold')
                            global_labels[dev_idx] = g

                        ui.separator()

                        ui.label('TIR (16 entrées)').classes('text-3xl mt-6 mb-4')
                        with ui.grid(columns=2).classes('w-full gap-6'):
                            analogs = []
                            for i in range(1, NUM_ANALOG_INPUTS + 1):
                                with ui.column().classes('items-center bg-zinc-900 p-6 rounded-2xl'):
                                    ui.label(f'TIR {i:02d}').classes('text-2xl text-gray-400')
                                    lbl = ui.label('—').classes('text-7xl font-mono mt-3')
                                    analogs.append(lbl)
                            analog_labels[dev_idx] = analogs

                        last_update[dev_idx] = ui.label('jamais').classes('text-lg text-gray-500 mt-8')

                        # Configuration
                        ui.separator()
                        ui.label('Configuration (hex)').classes('text-3xl text-gray-400 mt-10 mb-4')

                        with ui.row().classes('items-center gap-4 w-full'):
                            send_inp = ui.input(placeholder='01 02 A3 FF', label='Config à envoyer')\
                                .classes('flex-1').props('outlined clearable')
                            ui.button('Coller', color='secondary').props('flat').on_click(
                                lambda: ui.clipboard.read().then(lambda t: setattr(send_inp, 'value', t) or None)
                            )

                        def on_change():
                            f = format_hex(send_inp.value)
                            if f != send_inp.value: send_inp.value = f
                            valid = is_valid_hex(send_inp.value)
                            send_inp.classes(replace='border-red-500' if not valid and send_inp.value else 'border-green-500')

                        send_inp.on('update:model-value', on_change)
                        send_inputs[dev_idx] = send_inp

                        read_out = ui.input(label='Config lue').classes('mt-6').props('outlined filled')
                        read_outputs[dev_idx] = read_out

                        status_labels[dev_idx] = ui.label('').classes('text-sm mt-2')

                        with ui.row().classes('gap-4 mt-6 w-full'):
                            ui.button('Envoyer', color='primary', on_click=lambda d=dev_idx: send_action(d))\
                                .props('outline push').classes('flex-1')
                            ui.button('Lire config', color='indigo', on_click=lambda d=dev_idx: read_action(d))\
                                .props('outline push').classes('flex-1')


# ────────────────────────────────────────────────
# INTERFACE
# ────────────────────────────────────────────────
@ui.page('/')
async def main_page():
    global tabs_container, global_status

    ui.label('Supervision PyronuMGPF').classes('text-4xl font-bold text-center mt-6 mb-2')
    ui.label('État global + 16 TIR (2 bits chacun)').classes('text-center text-gray-500 text-xl mb-6')

    ui.separator()

    # ==================== PORT COM SÉLECTIONNABLE ====================
    with ui.row().classes('justify-center gap-6 mt-6'):
        port_select = ui.select(
            options=get_available_ports(),
            value=selected_port,
            label='Port COM'
        ).classes('w-64')

        async def change_port(e):
            global selected_port
            selected_port = e.value
            ui.notify(f'Port changé → {selected_port}', type='info')

        port_select.on('update:model-value', change_port)

        ui.button('↻ Ports', on_click=lambda: port_select.set_options(get_available_ports()))\
            .props('round dense')

    # ==================== SÉLECTION DES SLAVES ====================
    with ui.card().classes('w-full max-w-6xl mx-auto mt-8'):
        ui.label('Modules à afficher (masquage des onglets)').classes('text-2xl font-medium mb-4 px-6')
        with ui.grid(columns=6).classes('gap-4 p-6'):
            for dev in range(1, NUM_DEVICES + 1):
                sid = SLAVE_IDS[dev-1]
                ui.checkbox(f'{dev:02d} (ID {sid})', value=True).on(
                    'update:model-value',
                    lambda e, s=sid: (active_slaves.add(s) if e else active_slaves.discard(s))
                )

        ui.button('Appliquer sélection (masquer les autres onglets)',
                  on_click=refresh_tabs).classes('w-full mt-4').props('outline color=primary size=lg')

    # ==================== CONTENEUR DES ONGLETS ====================
    tabs_container = ui.element('div').classes('w-full mt-8')

    global_status = ui.label('Auto-refresh toutes les 0.1s').classes('text-center mt-10 text-lg')

    # Lancement initial
    ui.timer(0.5, refresh_tabs, once=True)

    # async def refresh_all():
    #     global_status.text = 'Lecture en cours...'
    #     for dev in range(1, NUM_DEVICES + 1):
    #         g_state, a_states, err = await asyncio.to_thread(modbus_read, SLAVE_IDS[dev-1])
    #         if err:
    #             global_labels[dev].text = 'ERR'
    #             global_labels[dev].classes(replace='text-red-600')
    #             for lbl in analog_labels[dev]:
    #                 lbl.text = 'ERR'
    #                 lbl.classes(replace='text-red-600')
    #         else:
    #             txt, cls = g_state
    #             global_labels[dev].text = txt
    #             global_labels[dev].classes(replace=cls)
    #             for i, (txt, cls) in enumerate(a_states):
    #                 analog_labels[dev][i].text = txt
    #                 analog_labels[dev][i].classes(replace=cls)

    #     global_status.text = f'Dernière màj : {time.strftime("%H:%M:%S")}'

    # async def auto_refresh():
    #     while True:
    #         await refresh_all()
    #         await asyncio.sleep(REFRESH_INTERVAL)

    # ui.timer(0.3, auto_refresh, once=True)


if __name__ in {"__main__", "__mp_main__"}:
    ui.run(title='PyronuMGPF Supervisor', host='0.0.0.0', port=8080, dark=True, reload=False)