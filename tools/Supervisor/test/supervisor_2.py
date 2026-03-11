#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Supervision PyronuMGPF - Version finale avec port COM + masquage onglets
"""

from nicegui import ui
from pymodbus.client import ModbusSerialClient as ModbusClient
import asyncio
import time
import re
import serial.tools.list_ports

# ===================== CONFIG =====================
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

# Variables dynamiques
selected_port = 'COM20'
active_slaves = set(SLAVE_IDS)          # slaves actuellement affichés

# Dictionnaires pour les composants (recréés à chaque régénération)
global_labels = {}
analog_labels = {}
last_update_labels = {}
send_inputs = {}
read_outputs = {}
status_labels = {}

tabs_container = None
global_status = None


def get_available_ports():
    ports = serial.tools.list_ports.comports()
    return [p.device for p in ports] or ["Aucun port détecté"]


# ===================== MODBUS =====================
def modbus_client():
    return ModbusClient(port=selected_port, baudrate=BAUDRATE, parity=PARITY,
                        stopbits=STOPBITS, bytesize=BYTESIZE, timeout=TIMEOUT)


def modbus_read(slave_id: int):
    client = modbus_client()
    try:
        if not client.connect():
            return None, None, "Connexion impossible"

        resp = client.read_holding_registers(GLOBAL_STATE_REG, 3, slave=slave_id)
        if resp.isError():
            return None, None, str(resp)

        regs = resp.registers
        if len(regs) != 3:
            return None, None, "Réponse incomplète"

        global_val = regs[0] & 0xFF
        global_state = GLOBAL_MAP.get(global_val, ("???", "text-3xl text-purple-600 font-bold"))

        low, high = regs[1], regs[2]
        bits32 = (high << 16) | low
        analogs = [ANALOG_MAP.get((bits32 >> (i*2)) & 0b11, DEFAULT_ANALOG) for i in range(16)]

        return global_state, analogs, None
    except Exception as e:
        return None, None, str(e)
    finally:
        client.close()


# (modbus_read_config et modbus_write_config restent identiques à ta version précédente)
# Je les ai gardées courtes ici pour la lisibilité, mais elles sont inchangées.

def modbus_read_config(slave_id: int, num_regs: int):
    client = modbus_client()
    try:
        if not client.connect(): return False, "Connexion impossible"
        resp = client.read_holding_registers(CONFIG_START_REG, num_regs, slave=slave_id)
        if resp.isError(): return False, str(resp)
        regs = resp.registers
        return True, ' '.join(f"{(v>>8)&0xFF:02X}{v&0xFF:02X}" for v in regs)
    except Exception as e:
        return False, str(e)
    finally:
        client.close()


async def modbus_write_config(slave_id: int, hex_string: str):
    cleaned = re.sub(r'[^0-9A-Fa-f]', '', hex_string.upper())
    if not cleaned or len(cleaned) % 2 != 0:
        return False, "Longueur hex invalide"
    try:
        bytes_data = bytes.fromhex(cleaned)
        values = [(bytes_data[i]<<8) | (bytes_data[i+1] if i+1 < len(bytes_data) else 0)
                  for i in range(0, len(bytes_data), 2)]
        client = modbus_client()
        try:
            if not client.connect(): return False, "Connexion impossible"
            result = client.write_registers(CONFIG_START_REG, values, slave=slave_id)
            return (False, str(result)) if result.isError() else (True, f"{len(values)} registres écrits")
        finally:
            client.close()
    except Exception as e:
        return False, str(e)


# ===================== UTILITAIRES =====================
def format_hex(value: str) -> str:
    cleaned = re.sub(r'[^0-9A-Fa-f]', '', value.upper())
    return ' '.join(cleaned[i:i+2] for i in range(0, len(cleaned), 2))

def is_valid_hex(value: str) -> bool:
    cleaned = re.sub(r'[^0-9A-Fa-f]', '', value.upper())
    return len(cleaned) % 2 == 0 and len(cleaned) > 0


# ===================== RECRÉATION DES ONGLETS =====================
def refresh_tabs():
    global tabs_container
    tabs_container.clear()

    if not active_slaves:
        with tabs_container:
            ui.label('Aucun module sélectionné').classes('text-4xl text-gray-500 text-center py-20')
        return

    with tabs_container:
        with ui.tabs().classes('w-full text-3xl') as tabs:
            tab_list = [ui.tab(f'PyronuMGPF {dev:02d} – ID {sid}')
                        for dev, sid in enumerate(SLAVE_IDS, 1) if sid in active_slaves]

        with ui.tab_panels(tabs, value=tab_list[0] if tab_list else None).classes('w-full'):
            for dev_idx, tab in enumerate(tab_list, start=1):
                sid = SLAVE_IDS[dev_idx-1]
                if sid not in active_slaves:
                    continue

                with ui.tab_panel(tab):
                    with ui.card().classes('w-full'):
                        ui.label(f'PyronuMGPF {dev_idx} • ID {sid}').classes('text-4xl font-bold mb-6')

                        # État global
                        with ui.row().classes('items-center gap-6 mb-8'):
                            ui.label('État global :').classes('text-3xl')
                            global_labels[dev_idx] = ui.label('—').classes('text-4xl font-bold')

                        ui.separator()

                        # 16 TIR en 2 colonnes
                        ui.label('TIR (16 entrées)').classes('text-3xl mt-6 mb-4')
                        with ui.grid(columns=2).classes('w-full gap-6'):
                            analogs = []
                            for i in range(1, 17):
                                with ui.column().classes('items-center bg-zinc-900 p-6 rounded-2xl w-full'):
                                    ui.label(f'TIR {i:02d}').classes('text-2xl text-gray-400')
                                    lbl = ui.label('—').classes('text-7xl font-mono mt-3')
                                    analogs.append(lbl)
                            analog_labels[dev_idx] = analogs

                        last_update_labels[dev_idx] = ui.label('jamais').classes('text-lg text-gray-500 mt-8')

                        # Configuration
                        ui.separator()
                        ui.label('Configuration (hex)').classes('text-3xl text-gray-400 mt-10 mb-4')

                        with ui.row().classes('items-center gap-4 w-full'):
                            send_inp = ui.input(placeholder='ex: 01 02 A3 FF', label='Config à envoyer')\
                                .classes('flex-1').props('outlined clearable')
                            ui.button('Coller', color='secondary').props('flat').on_click(
                                lambda e, inp=send_inp: ui.clipboard.read().then(lambda t: setattr(inp, 'value', t) or None)
                            )

                        # ... (le reste de la config reste identique à ta version)

    ui.notify(f"{len(active_slaves)} modules affichés", type='positive')


# ===================== PAGE =====================
@ui.page('/')
async def main_page():
    global tabs_container, global_status

    # Header
    ui.label('Supervision PyronuMGPF').classes('text-5xl font-bold text-center mt-6')

    # Port COM
    with ui.row().classes('justify-center gap-6 mt-6'):
        ports = get_available_ports()
        port_select = ui.select(ports, value=selected_port, label='Port COM').classes('w-64')
        port_select.on('update:model-value', lambda e: globals().update(selected_port=e.value))

        ui.button('↻ Ports', on_click=lambda: port_select.set_options(get_available_ports())).props('round')

    # Sélection slaves
    with ui.card().classes('w-full max-w-6xl mx-auto mt-8'):
        ui.label('Modules à afficher').classes('text-2xl font-medium mb-4 px-6')
        with ui.grid(columns=6).classes('gap-4 p-6'):
            for dev in range(1, NUM_DEVICES + 1):
                sid = SLAVE_IDS[dev-1]
                ui.checkbox(f'{dev:02d} (ID {sid})', value=True).on('update:model-value',
                    lambda e, s=sid: (active_slaves.add(s) if e else active_slaves.discard(s)))

        ui.button('Appliquer sélection (masquer les autres)', on_click=refresh_tabs)\
            .classes('w-full mt-4').props('outline color=primary size=lg')

    # Conteneur des onglets
    tabs_container = ui.element('div').classes('w-full mt-8')

    global_status = ui.label('Auto-refresh toutes les 0.1s').classes('text-center mt-10 text-xl')

    # Lancement
    ui.timer(0.5, refresh_tabs, once=True)

    async def auto_refresh():
        while True:
            await refresh_all()
            await asyncio.sleep(REFRESH_INTERVAL)

    ui.timer(0.3, auto_refresh, once=True)


if __name__ in {"__main__", "__mp_main__"}:
    ui.run(title='PyronuMGPF Supervisor', host='0.0.0.0', port=8080, dark=True, reload=False)