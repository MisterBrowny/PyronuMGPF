#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Supervision Modbus RTU - 20 esclaves
État global (1 octet LSB) + 16 entrées analogiques (2 bits chacune, LSB first)
+ Deux champs distincts : config à envoyer + config lue
Version NiceGUI actuelle (tab + tab_panel)
"""

from nicegui import ui
from pymodbus.client import ModbusSerialClient as ModbusClient
import asyncio
import time
import re

# ────────────────────────────────────────────────
# CONFIGURATION (à personnaliser !)
# ────────────────────────────────────────────────
PORT = 'COM20'           # ← À CHANGER
BAUDRATE = 9600
PARITY = 'N'
STOPBITS = 1
BYTESIZE = 8
TIMEOUT = 0.1

NUM_DEVICES = 20
NUM_ANALOG_INPUTS = 16

GLOBAL_STATE_REG   = 0       # registre état global (LSB)
ANALOG_START_REG   = 1       # 2 registres → 32 bits analogiques
CONFIG_START_REG   = 3       # ← À CHANGER : adresse de départ config
CONFIG_NUM_REGS    = 8         # ← À CHANGER : combien de registres lire/écrire

SLAVE_IDS = list(range(1, NUM_DEVICES + 1))

REFRESH_INTERVAL = 0.1

GLOBAL_MAP = {
    0: ("END",    "text-2xl text-gray-700 font-bold"),
    1: ("GO",     "text-2xl text-green-600 font-bold"),
    2: ("ARMED",  "text-2xl text-blue-600 font-bold"),
    3: ("TEST",   "text-2xl text-orange-600 font-bold"),
}

ANALOG_MAP = {
    0b00: ("ABSENT", "text-2xl text-gray-500"),
    0b01: ("KO",     "text-2xl text-red-600 font-bold"),
    0b10: ("MOYEN",  "text-2xl text-yellow-600 font-bold"),
    0b11: ("OK",     "text-2xl text-green-600 font-bold"),
}

DEFAULT_GLOBAL = ("???", "text-2xl text-purple-600 font-bold")
DEFAULT_ANALOG = ("ERR", "text-2xl text-red-600 font-bold")


# ────────────────────────────────────────────────
# FONCTIONS MODBUS
# ────────────────────────────────────────────────
def modbus_read(slave_id: int):
    client = ModbusClient(port=PORT, baudrate=BAUDRATE,
                          parity=PARITY, stopbits=STOPBITS, bytesize=BYTESIZE,
                          timeout=TIMEOUT)
    try:
        if not client.connect():
            return None, None, "Connexion impossible"
        
        # print("ICI")

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
    client = ModbusClient(port=PORT, baudrate=BAUDRATE,
                          parity=PARITY, stopbits=STOPBITS, bytesize=BYTESIZE,
                          timeout=TIMEOUT)
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

        client = ModbusClient(method='rtu', port=PORT, baudrate=BAUDRATE,
                              parity=PARITY, stopbits=STOPBITS, bytesize=BYTESIZE,
                              timeout=TIMEOUT)
        try:
            if not client.connect():
                return False, "Connexion impossible"

            result = client.write_registers(CONFIG_START_REG, values, slave=slave_id)
            if result.isError():
                return False, str(result)
            return True, f"{len(values)} registres écrits"
        finally:
            client.close()

    except Exception as e:
        return False, str(e)


# ────────────────────────────────────────────────
# UTILITAIRES HEX
# ────────────────────────────────────────────────
def format_hex(value: str) -> str:
    cleaned = re.sub(r'[^0-9A-Fa-f]', '', value.upper())
    return ' '.join(cleaned[i:i+2] for i in range(0, len(cleaned), 2))


def is_valid_hex(value: str) -> bool:
    cleaned = re.sub(r'[^0-9A-Fa-f]', '', value.upper())
    return len(cleaned) % 2 == 0 and len(cleaned) > 0


# ────────────────────────────────────────────────
# INTERFACE
# ────────────────────────────────────────────────
@ui.page('/')
async def main_page():
    ui.label('Supervision PyronuMGPF').classes('text-2xl font-bold text-center mt-4 mb-1')
    ui.label('État global + 16×2 bits + config (envoi / lecture séparés)').classes('text-center text-gray-600 mb-3')

    ui.separator()

    # Création des onglets
    with ui.tabs().classes('w-full text-3xl') as tabs:
        tab_list = []
        for dev in range(1, NUM_DEVICES + 1):
            sid = SLAVE_IDS[dev - 1]
            tab_list.append(ui.tab(f'PyronuMGPF {dev:02d} – ID {sid}'))

    with ui.tab_panels(tabs, value=tab_list[0]).classes('w-full'):
        global_labels = {}
        analog_labels = {}
        last_update = {}
        send_inputs = {}
        read_outputs = {}
        status_labels = {}

        for dev_idx, tab in enumerate(tab_list, start=1):
            sid = SLAVE_IDS[dev_idx - 1]

            with ui.tab_panel(tab):
                with ui.card().classes('w-full'):
                    ui.label(f'PyronuMGPF {dev_idx} • Id {sid}').classes('text-3xl font-semibold mb-4')

                    # État global
                    with ui.row().classes('items-center gap-4 mb-5'):
                        ui.label('État :').classes('text-3xl')
                        g = ui.label('—').classes('text-3xl font-bold')
                        global_labels[dev_idx] = g

                    ui.separator()

                    # Test Infla
                    ui.label('Test Infla').classes('text-3xl mt-4 mb-2')
                    # with ui.grid(columns={'default': 2, 'sm': 3, 'md': 4}).classes('gap-3 w-full'):
                    #     analogs = []
                    #     for i in range(1, NUM_ANALOG_INPUTS + 1):
                    #         with ui.column().classes('items-center bg-gray-50 p-3 rounded'):
                    #             ui.label(f'IN {i:02d}').classes('text-sm text-gray-700')
                    #             lbl = ui.label('—').classes('text-xl font-mono mt-1')
                    #             analogs.append(lbl)
                    #     analog_labels[dev_idx] = analogs
                    # with ui.grid(columns=2).classes('gap-3 w-full'):
                    #     analogs = []
                    #     # colonne des entrées impaires
                    #     with ui.column().classes('gap-3'):
                    #         for i in range(1, NUM_ANALOG_INPUTS + 1, 2):
                    #             with ui.column().classes('items-center bg-gray-50 p-3 rounded'):
                    #                 ui.label(f'IN {i:02d}').classes('text-sm text-gray-700')
                    #                 lbl = ui.label('—').classes('text-xl font-mono mt-1')
                    #                 analogs.append(lbl)

                    #     # colonne des entrées paires
                    #     with ui.column().classes('gap-3'):
                    #         for i in range(2, NUM_ANALOG_INPUTS + 1, 2):
                    #             with ui.column().classes('items-center bg-gray-50 p-3 rounded'):
                    #                 ui.label(f'IN {i:02d}').classes('text-sm text-gray-700')
                    #                 lbl = ui.label('—').classes('text-xl font-mono mt-1')
                    #                 analogs.append(lbl)
                    #     analog_labels[dev_idx] = analogs
                    # with ui.row().classes('w-full gap-4'):
                    #     analogs = [None] * NUM_ANALOG_INPUTS
                    #     # colonne impaire
                    #     with ui.column().classes('w-1/2 gap-3'):
                    #         for i in range(1, NUM_ANALOG_INPUTS + 1, 2):
                    #             with ui.column().classes('items-center bg-gray-50 p-3 rounded w-full'):
                    #                 ui.label(f'IN {i:02d}').classes('text-sm text-gray-700')
                    #                 lbl = ui.label('—').classes('text-xl font-mono mt-1')
                    #                 analogs[i-1] = lbl
                    #     # colonne paire
                    #     with ui.column().classes('w-1/2 gap-3'):
                    #         for i in range(2, NUM_ANALOG_INPUTS + 1, 2):
                    #             with ui.column().classes('items-center bg-gray-50 p-3 rounded w-full'):
                    #                 ui.label(f'IN {i:02d}').classes('text-sm text-gray-700')
                    #                 lbl = ui.label('—').classes('text-xl font-mono mt-1')
                    #                 analogs[i-1] = lbl
                    #     analog_labels[dev_idx] = analogs
                    with ui.grid(columns=2).classes('w-full gap-4 shadow-sm'):

                        analogs = [None] * NUM_ANALOG_INPUTS

                        for i in range(1, NUM_ANALOG_INPUTS + 1):
                            with ui.column().classes('items-center bg-gray-50 p-3 rounded w-full'):
                                ui.label(f'TIR {i:02d}').classes('text-3xl text-gray-700')
                                lbl = ui.label('—').classes('text-6xl font-mono mt-1')
                                analogs[i-1] = lbl

                        analog_labels[dev_idx] = analogs

                    last_update[dev_idx] = ui.label('jamais').classes('text-sm text-gray-500 mt-5 italic text-center')

                    # Configuration - deux champs
                    ui.separator()
                    ui.label('Configuration (hex)').classes('text-sm text-gray-600 mt-6 mb-2')

                    # Champ ENVOI
                    with ui.row().classes('items-center gap-3 w-full'):
                        send_inp = ui.input(
                            placeholder='ex: 01 02 A3 FF',
                            label='Config à envoyer'
                        ).classes('flex-grow').props('outlined clearable')

                        async def paste_to_send():
                            try:
                                text = await ui.clipboard.read()
                                if text:
                                    send_inp.value = text
                                    ui.notify('Collé dans "à envoyer"', type='positive')
                            except:
                                ui.notify('Impossible de coller', type='negative')

                        ui.button('Coller', on_click=paste_to_send, color='secondary')\
                            .props('flat dense').classes('px-4')

                    def on_send_change():
                        formatted = format_hex(send_inp.value)
                        if formatted != send_inp.value:
                            send_inp.value = formatted
                        valid = is_valid_hex(send_inp.value)
                        send_inp.classes(replace='border-red-500' if not valid and send_inp.value else 'border-green-500' if valid and send_inp.value else '')

                    send_inp.on('update:model-value', on_send_change)
                    send_inputs[dev_idx] = send_inp

                    # Champ LECTURE
                    read_out = ui.input(
                        label='Config lue'#,
                        #readonly=True
                    ).classes('mt-3').props('outlined filled')

                    read_outputs[dev_idx] = read_out

                    status_lbl = ui.label('').classes('text-xs mt-1 min-h-5')
                    status_labels[dev_idx] = status_lbl

                    # Boutons
                    with ui.row().classes('gap-3 mt-4 w-full'):
                        async def send_action(dev_id=dev_idx):
                            val = send_inputs[dev_id].value.strip()
                            if not val:
                                ui.notify('Rien à envoyer', type='warning')
                                return
                            success, msg = await modbus_write_config(SLAVE_IDS[dev_id-1], val)
                            if success:
                                ui.notify(msg, type='positive')
                                send_inputs[dev_id].value = ''
                                status_labels[dev_id].text = f'Envoyé – {time.strftime("%H:%M:%S")}'
                                status_labels[dev_id].classes(replace='text-green-600')
                            else:
                                ui.notify(f'Échec : {msg}', type='negative')
                                status_labels[dev_id].text = f'Échec envoi : {msg[:50]}'
                                status_labels[dev_id].classes(replace='text-red-600')

                        ui.button('Envoyer', on_click=send_action, color='primary')\
                            .props('outline push').classes('flex-grow')

                        async def read_action(dev_id=dev_idx):
                            success, result = await asyncio.to_thread(
                                modbus_read_config,
                                SLAVE_IDS[dev_id-1],
                                CONFIG_NUM_REGS
                            )
                            if success:
                                read_outputs[dev_id].value = result
                                status_labels[dev_id].text = f'Lu – {time.strftime("%H:%M:%S")}'
                                status_labels[dev_id].classes(replace='text-blue-600')
                                ui.notify('Configuration lue avec succès', type='positive')
                            else:
                                read_outputs[dev_id].value = ''
                                status_labels[dev_id].text = f'Échec lecture : {result[:50]}'
                                status_labels[dev_id].classes(replace='text-red-600')
                                ui.notify(f'Échec : {result}', type='negative')

                        ui.button('Lire config', on_click=read_action, color='indigo')\
                            .props('outline push').classes('flex-grow')

    ui.separator()
    global_status = ui.label('Auto-refresh toutes les 5 s').classes('text-center mt-4 mb-2')

    async def refresh_all():
        global_status.text = 'Lecture états en cours...'
        #await ui.context.client.request_refresh()

        has_err = False
        now_str = time.strftime('%H:%M:%S')

        for dev in range(1, NUM_DEVICES + 1):
            g_state, a_states, err = await asyncio.to_thread(modbus_read, SLAVE_IDS[dev-1])

            if err:
                has_err = True
                global_labels[dev].text = 'ERR'
                global_labels[dev].classes(replace='text-red-600')
                for lbl in analog_labels[dev]:
                    lbl.text = 'ERR'
                    lbl.classes(replace='text-red-600')
                last_update[dev].text = f'erreur {now_str}'
                last_update[dev].classes(replace='text-red-600')
            else:
                txt, cls = g_state
                global_labels[dev].text = txt
                global_labels[dev].classes(replace=f'text-{cls}')

                for i, (txt, cls) in enumerate(a_states):
                    analog_labels[dev][i].text = txt
                    analog_labels[dev][i].classes(replace=cls)

                last_update[dev].text = f'màj {now_str}'
                last_update[dev].classes(replace='text-gray-600')

        global_status.text = f'Dernière màj états : {now_str}  {"• erreurs détectées" if has_err else ""}'
        global_status.classes(replace='text-red-600' if has_err else 'text-gray-700')

    ui.button('Rafraîchir états maintenant', on_click=refresh_all)\
        .props('outline color=primary size=lg').classes('mx-auto mt-3 block')

    async def auto_refresh():
        while True:
            await refresh_all()
            await asyncio.sleep(REFRESH_INTERVAL)

    ui.timer(1.0, auto_refresh, once=True)


if __name__ in {"__main__", "__mp_main__"}:
    ui.run(
        title='MGPF',
        host='0.0.0.0',
        port=8080,
        dark=True,
        reload=True,
    )