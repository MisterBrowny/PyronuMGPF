#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Supervision Modbus RTU - 20 esclaves
État global (1 octet LSB) + 16 entrées analogiques (2 bits chacune, LSB first)
+ Deux champs distincts : config à envoyer + config lue
Version NiceGUI actuelle (tab + tab_panel)
"""

from nicegui import ui, app
from pymodbus.client import ModbusSerialClient as ModbusClient
import asyncio
import time
import re
import serial.tools.list_ports 
import logging
import sys
import multiprocessing
import os
import threading
import socket

from collections import deque

hostname = socket.gethostname()
ip = socket.gethostbyname(hostname)

url = f"http://{ip}:8080"

if sys.stdout is None:
    sys.stdout = open("nul", "w")
if sys.stderr is None:
    sys.stderr = open("nul", "w")

logging.getLogger("pymodbus").setLevel(logging.CRITICAL)
logging.getLogger('watchfiles').setLevel(logging.CRITICAL)
logging.getLogger('asyncio').setLevel(logging.CRITICAL)
logging.getLogger('nicegui').setLevel(logging.CRITICAL)

# ────────────────────────────────────────────────
# CONFIGURATION (à personnaliser !)
# ────────────────────────────────────────────────
selected_port = None
BAUDRATE = 9600
PARITY = 'N'
STOPBITS = 1
BYTESIZE = 8
TIMEOUT = 0.06

REFRESH_INTERVAL = 0

# PYRO MODBUS REGISTER
NUM_DEVICES = 20
NUM_ANALOG_INPUTS = 16

GLOBAL_STATE_REG   = 0       # registre état global (LSB)
ANALOG_START_REG   = 1       # 2 registres → 32 bits analogiques
ALIM_START_REG     = 3
ALIM_1A_START_REG  = 4
CONFIG_START_REG   = 5       # ← À CHANGER : adresse de départ config
CONFIG_NUM_REGS    = 55      # ← À CHANGER : combien de registres lire/écrire

SLAVE_IDS = list(range(1, NUM_DEVICES + 1))
active_slaves = set(SLAVE_IDS)

GLOBAL_MAP = {
    0: ("DECO",  "text-3xl text-red-700 font-bold"), # PAS UTILS ?
    1: ("END",   "text-3xl text-yellow-700 font-bold animate-pulse"),
    2: ("GO",    "text-3xl text-red-600 font-bold animate-pulse"),
    3: ("ARMED", "text-3xl text-orange-500 font-bold animate-pulse"),
    4: ("TEST",  "text-3xl text-green-600 font-bold animate-pulse"),
    5: ("PROG",  "text-3xl text-sky-800 font-bold animate-pulse"),
    6: ("STOP",  "text-3xl text-orange-400 font-bold animate-pulse"),
    7: ("PAUSE",  "text-3xl text-orange-300 font-bold animate-pulse"),
}

ANALOG_MAP = {
    0b00: ("ABSENT", "text-2xl text-gray-500"),
    0b01: ("KO",     "text-2xl text-red-600 font-bold"),
    0b10: ("MOYEN",  "text-2xl text-yellow-600 font-bold"),
    0b11: ("OK",     "text-2xl text-green-600 font-bold"),
}

DEFAULT_GLOBAL = ("???", "text-3xl text-gray-500 font-bold")
DEFAULT_ANALOG = ("—", "text-2xl font-bold")
DEFAULT_ALIM = ("—")

DEFAULT_TIME = ("never", "text-base text-gray-500 mt-0 italic text-center")

def get_available_ports():
    """Retourne la liste des ports COM disponibles"""
    ports = serial.tools.list_ports.comports()
    return [p.device for p in ports] or ["Aucun port détecté"]

device_states = {
    i: {
        "global": DEFAULT_GLOBAL,
        "analogs": [DEFAULT_ANALOG] * NUM_ANALOG_INPUTS,
        "alim": DEFAULT_ALIM,
        "alim_1A": DEFAULT_ALIM,
        "no_refresh": True,
        "disconnected": False,
        "last_request": DEFAULT_TIME,
        "last_response": DEFAULT_TIME,
        "dirty": True,
    }
    for i in SLAVE_IDS
}

# BATTERY MODBUS REGISTER
BATTERY_START_REG = 0
LIST_BAT_CELLS = {
    0: "CELL 1 & 2 (x2)",
    1: "CELL 3",
    2: "CELL 4",
    3: "CELL 5 & 6 (x2)",
    4: "CELL 7",
    5: "CELL 8",
    6: "CELL 9",
    7: "CELL 10",
}
NUM_BAT_CELLS = len(LIST_BAT_CELLS)
HISTORY_SIZE = 300  # environ 5 min à 1 échantillon/s

BATTERY_REG_COUNT = 10

BATTERY_IDS = list(range(200, 221))

BATTERY_STATE_MAP = {
    0: ('DISCONNECT', 'text-3xl text-red-700 font-bold'),
    1: ('LOW BATTERY VOLTAGE', 'text-3xl text-red-900 font-bold'),
    2: ('LOW CELL VOLTAGE', 'text-3xl text-orange-600 font-bold'),
    3: ('NOMINAL VOLTAGE', 'text-3xl text-green-600 font-bold'),
}

DEFAULT_BATTERY_STATE = (
    'UNKNOWN',
    'text-gray-500 font-bold'
)

battery_states = {
    i: {
        "global": DEFAULT_GLOBAL,
        "pack": DEFAULT_ALIM,
        "cell": [DEFAULT_ALIM] * NUM_BAT_CELLS,
        "no_refresh": True,
        "disconnected": False,
        "last_request": DEFAULT_TIME,
        "last_response": DEFAULT_TIME,
        "dirty": True,
    }
    for i in BATTERY_IDS
}

cell_history = {
    sid: [
        deque(maxlen=HISTORY_SIZE)
        for _ in range(NUM_BAT_CELLS)
    ]
    for sid in BATTERY_IDS
}

# =========================================================
# ONGLETS PYRO + BATTERIES AVEC COULEURS DYNAMIQUES
# =========================================================

# stockage références onglets
pyro_tabs = {}
battery_tabs_dict = {}

# état actuel des couleurs
pyro_tab_states = {}
battery_tab_states = {}

# =========================================================
# MAPPING COULEURS
# =========================================================

PYRO_TAB_COLORS = {
    'DECO': 'bg-gray-700 text-white',
    'END': 'bg-yellow-700 text-white',
    'GO': 'bg-red-600 text-white',
    'ARMED': 'bg-orange-500 text-white',
    'TEST': 'bg-green-600 text-white',
    'PROG': 'bg-sky-800 text-white',
    'STOP': 'bg-orange-400 text-white',
    'PAUSE': 'bg-orange-300 text-white'
}

BATTERY_TAB_COLORS = {
    'DISCONNECT': 'bg-gray-700 text-white',
    'LOW BATTERY VOLTAGE': 'bg-red-900 text-white',
    'LOW CELL VOLTAGE': 'bg-orange-600 text-white',
    'NOMINAL VOLTAGE': 'bg-green-700 text-white',
    'UNKNOWN': 'bg-gray-700 text-white',
}

ALL_TAB_CLASSES = (
    'bg-red-700 bg-yellow-700 bg-orange-600 '
    'bg-green-700 bg-blue-700 bg-gray-700 '
    'bg-red-900 bg-orange-700 bg-yellow-600 '
    'text-white text-black'
)

modbus_client = None
modbus_running = True
modbus_polling = True
last_connection_attempt = 0
modbus_lock = threading.Lock()

# ────────────────────────────────────────────────
# FONCTIONS MODBUS
# ────────────────────────────────────────────────

def modbus_read_pyro(client, sid):

    try:
        device_states[sid]["last_request"] = (time.strftime('%H:%M:%S'), "text-base text-gray-500 mt-0 italic text-center")

        resp = client.read_holding_registers(
            GLOBAL_STATE_REG,
            count=5,
            device_id=sid
        )
        
        if resp.isError():
            device_states[sid]["no_refresh"] = False
            device_states[sid]["disconnected"] = True
            return

        regs = resp.registers

        global_val = regs[0] & 0xFF
        global_state = GLOBAL_MAP.get(global_val, DEFAULT_GLOBAL)

        low = regs[1]
        high = regs[2]

        bits32 = (high << 16) | low

        analogs = []

        for i in range(NUM_ANALOG_INPUTS):

            val2 = (bits32 >> (i * 2)) & 0b11
            analogs.append(ANALOG_MAP.get(val2, DEFAULT_ANALOG))

        device_states[sid]["global"] = global_state
        device_states[sid]["analogs"] = analogs
        device_states[sid]["alim"] = regs[3]
        device_states[sid]["alim_1A"] = regs[4]
        device_states[sid]["no_refresh"] = False
        device_states[sid]["disconnected"] = False
        device_states[sid]["last_response"] = (time.strftime('%H:%M:%S'), "text-base text-gray-500 mt-0 italic text-center")
        device_states[sid]["dirty"] = True

    except Exception:
        device_states[sid]["no_refresh"] = False
        device_states[sid]["disconnected"] = True

def modbus_read_battery(client, sid):

    try:
        battery_states[sid]["last_request"] = (time.strftime('%H:%M:%S'), "text-base text-gray-500 mt-0 italic text-center")

        resp = client.read_holding_registers(
            address=BATTERY_START_REG,
            count=BATTERY_REG_COUNT,
            device_id=sid
        )

        if resp.isError():
            battery_states[sid]["no_refresh"] = False
            battery_states[sid]["disconnected"] = True
            return

        regs = resp.registers

        bat_val = regs[0] & 0xFF
        bat_state = BATTERY_STATE_MAP.get(bat_val, DEFAULT_BATTERY_STATE)

        pack_voltage = regs[1]
        cell_voltages = regs[2:10]

        battery_states[sid]["global"] = bat_state
        battery_states[sid]["pack"] = pack_voltage
        battery_states[sid]["cell"] = cell_voltages
        battery_states[sid]["no_refresh"] = False
        battery_states[sid]["disconnected"] = False
        battery_states[sid]["last_response"] = (time.strftime('%H:%M:%S'), "text-base text-gray-500 mt-0 italic text-center")
        battery_states[sid]["dirty"] = True

    except Exception:
        battery_states[sid]["no_refresh"] = False
        battery_states[sid]["disconnected"] = True

    
def modbus_worker():

    global modbus_client, last_connection_attempt
    
    while modbus_running:
        if selected_port is None:
            time.sleep(1)
            continue

        try:
            if modbus_client is None or not modbus_client.connected:
                modbus_client = ModbusClient(
                    port=selected_port,
                    baudrate=BAUDRATE,
                    parity=PARITY,
                    stopbits=STOPBITS,
                    bytesize=BYTESIZE,
                    timeout=TIMEOUT,
                    retries=0
                )
                modbus_client.connect()

            if modbus_client.connected:
                for sid in list(active_slaves):
                    with modbus_lock:
                        modbus_read_pyro(modbus_client, sid)

                    time.sleep(0.002)
                
                for sid in BATTERY_IDS:
                    with modbus_lock:
                        modbus_read_battery(modbus_client, sid)
                    
                    time.sleep(0.002)

        except Exception as e:
            print(f"[WORKER] {e}")

            try:
                modbus_client.close()
            except:
                pass

            modbus_client = None

        time.sleep(REFRESH_INTERVAL)


def modbus_read_config(slave_id: int, num_regs: int):
    global modbus_client
    if modbus_client is None or not modbus_client.connected:
        return False, "Connexion impossible"
    else:
        # Utilise le client du worker si disponible
        try:
            resp = modbus_client.read_holding_registers(CONFIG_START_REG, count=num_regs, device_id=slave_id)
            if resp.isError():
                return False, str(resp)
            
            regs = resp.registers
            octets = []

            # Conversion de tous les registres en octets
            for val in regs:
                octets.append(f"{val & 0xFF:02X}")
                octets.append(f"{(val >> 8) & 0xFF:02X}")
                
            # Création des blocs de 4 octets avec tiret entre le 1er et le 2ème
            blocks = []
            for i in range(0, len(octets)-6, 4):
                if i + 3 < len(octets):
                    block = f"{octets[i]}-{octets[i+1]}{octets[i+2]}{octets[i+3]}"
                blocks.append(block)
            i = i + 4
            block = f"{octets[i]}{octets[i+1]}-{octets[i+2]}{octets[i+3]}{octets[i+4]}{octets[i+5]}"
            blocks.append(block)
            # Séparation des blocs par plusieurs espaces
            formatted = ' '.join(blocks)

            return True, formatted
            
        except Exception as e:
            return False, str(e)


async def modbus_write_config(slave_id: int, hex_string: str):
    global modbus_client
    
    # print(f"[WRITE] Début envoi vers slave {slave_id} | Données brutes: '{hex_string}'")
    
    if modbus_client is None:
        # print("[WRITE] modbus_client is none")
        return False, "Connexion impossible" 
    elif modbus_client.connected == False:
        # print("[WRITE] modbus_client not connected")
        return False, "Connexion impossible"    
    else:

        if not hex_string or not isinstance(hex_string, str):
            # print("[WRITE] ERREUR: hex_string est vide ou None")
            return False, "Aucune donnée à envoyer"

        cleaned = re.sub(r'[^0-9A-Fa-f]', '', hex_string.upper().strip())
        # print(f"[WRITE] Données nettoyées: {cleaned} ({len(cleaned)} caractères)")
        if not cleaned or len(cleaned) % 2 != 0:
            # print("[WRITE] ERREUR: Longueur invalide")
            return False, "Données hex invalides"

        try:
            bytes_data = bytes.fromhex(cleaned)
            values = []
            for i in range(0, len(bytes_data), 2):
                hi = bytes_data[i]
                lo = bytes_data[i + 1] if i + 1 < len(bytes_data) else 0
                values.append((lo << 8) | hi)

            # print(f"[WRITE] {len(values)} registres à écrire → {values}")
            with modbus_lock:

                modbus_client.write_registers(
                    CONFIG_START_REG,
                    values,
                    device_id=slave_id,
                    no_response_expected=True
                )
                await asyncio.sleep(0.5)
            
            # print(f"[WRITE] Succès sur slave {slave_id}")
            return True, f"{len(values)} registres écrits avec succès"
            
        except Exception as e:
            # print(f"[WRITE] EXCEPTION: {type(e).__name__} - {e}")
            return False, f"Exception: {str(e)}"

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

def resource_path(relative_path):
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)

def toggle_all_slave(e):
    for sid in SLAVE_IDS:
        if e.value:
            active_slaves.add(sid)
        else:
            active_slaves.discard(sid)
            device_states[sid]["no_refresh"] = True

# =========================================================
# FONCTIONS MAJ COULEURS ONGLETS
# =========================================================

def update_pyro_tab_color(sid, state):
    if pyro_tab_states[sid] == state:
        return

    pyro_tab_states[sid] = state
    color = PYRO_TAB_COLORS.get(state, 'bg-gray-700 text-white')
    pyro_tabs[sid].classes(remove=ALL_TAB_CLASSES)
    pyro_tabs[sid].classes(add=f'{color} rounded-xl shadow font-bold')

def update_battery_tab_color(sid, state):
    if battery_tab_states[sid] == state:
        return

    battery_tab_states[sid] = state
    color = BATTERY_TAB_COLORS.get(state, 'bg-gray-700 text-white')
    battery_tabs_dict[sid].classes(remove=ALL_TAB_CLASSES)
    battery_tabs_dict[sid].classes(add=f'{color} rounded-xl shadow font-bold')

@ui.page('/')

async def main_page():
    asset_path = resource_path('asset')

    app.add_static_files('/asset', asset_path)
    
    #i.query('body').classes('max-w-screen-md md:max-w-screen-lg xl:max-w-screen-2xl mx-auto')

    ui.add_head_html('<link rel="icon" href="/asset/icon.ico">')

    with ui.row().classes('text-stretch gap-10 mb-2'):
        ui.image('/asset/logo_mgpf.png').classes('w-40 rounded shadow')
        ui.label('Supervisor').classes('mt-8 text-red-800 text-6xl font-bold text-center')
    
    # ==================== PORT COM SÉLECTIONNABLE ====================
    
        port_select = ui.select(
            options=get_available_ports(),
            value=selected_port,
            label='Port COM'
        ).classes('mt-9 text-center w-32')

        async def change_port(e):
            global selected_port
            selected_port = e.value
            ui.notify(f'Port changé → {selected_port}', type='info')

        port_select.on_value_change(change_port)

        ui.button(
            '',
            icon='refresh',
            on_click=lambda: port_select.set_options(get_available_ports())
        ).props('rounded-lg unelevated color=primary').classes('mt-12 text-center px-4 py-2 shadow-md ')

        ui.checkbox(f'Active refresh', value=True).classes('mt-12 text-1xl font-semibold').on_value_change(lambda e: toggle_all_slave(e))

    
    # ==================== Création des onglets ====================
    # with ui.tabs().classes('w-full text-3xl') as tabs:
    #     tab_list = []
    #     for dev in range(1, NUM_DEVICES + 1):
    #         sid = SLAVE_IDS[dev - 1]
    #         tab_list.append(ui.tab(f'PYRO {sid:02d}'))

    # =========================================================
    # CREATION ONGLETS PYRO
    # =========================================================

    with ui.tabs().classes('w-full text-lg') as pyro_tabs_bar:
        pyro_tab_list = []

        for sid in SLAVE_IDS:
            tab = ui.tab(f'PYRO {sid:02d}') \
                .classes(
                    'rounded-xl shadow font-bold transition-all duration-300'
                )
            pyro_tabs[sid] = tab
            pyro_tab_states[sid] = 'INIT'
            pyro_tab_list.append(tab)

    # =========================================================
    # PANELS PYRO
    # =========================================================

    with ui.tab_panels(pyro_tabs_bar, value=pyro_tab_list[0]).classes('w-full mx-auto shadow-lg rounded-xl px-4'):
        global_labels = {}
        analog_labels = {}
        alim_labels = {}
        alim_1A_labels = {}
        last_request = {}
        last_response = {}
        send_inputs = {}
        read_outputs = {}
        status_labels = {}
        memo_labels = {}
        memo_state = {}

        for dev_idx, tab in enumerate(pyro_tab_list, start=1):
            sid = SLAVE_IDS[dev_idx - 1]

            with ui.tab_panel(tab):
                with ui.card().classes('w-full'):
                    with ui.row().classes('items-center gap-6 mb-2'):
                        ui.label(f'PyronuMGPF {sid:02d}').classes('text-3xl font-semibold mb-2')
                        # ui.checkbox(f'Active refresh', value=False).classes('text-1xl font-semibold').on_value_change(lambda e, s=sid: toggle_slave(e, s))

                    # État global
                    with ui.row().classes('items-center gap-6 mb-0'):
                        ui.label('  Statut :').classes('text-3xl')
                        g = ui.label('—').classes('text-4xl font-bold')
                        global_labels[dev_idx] = g

                        ui.label(' / ALIM :').classes('text-3xl')
                        a = ui.label('—').classes('text-4xl font-bold')
                        alim_labels[dev_idx] = a

                        ui.label('mV  / ALIM (1A) :').classes('text-3xl')
                        a1A = ui.label('—').classes('text-4xl font-bold')
                        alim_1A_labels[dev_idx] = a1A

                        ui.label('mV').classes('text-3xl')

                    ui.separator()

                    # Test Infla
                    ui.label('Test Infla').classes('text-2xl mt-0 mb-2')
 
                    with ui.grid(columns=2).classes('w-full gap-6'):

                        analogs = [None] * NUM_ANALOG_INPUTS

                        for i in range(1, NUM_ANALOG_INPUTS + 1):
                            with ui.column().classes('items-center bg-zinc-700 p-1 rounded-3xl w-auto'):
                                ui.label(f'TIR {i:02d}').classes('text-1xl text-white-500')
                                lbl = ui.label('—').classes('text-2xl font-bold')
                                analogs[i-1] = lbl

                        analog_labels[dev_idx] = analogs

                    with ui.row().classes('items-center w-full'):
                        ui.label('Last request time: ').classes('text-base text-gray-500 mt-0 italic text-center')
                        last_request[dev_idx] = ui.label('never').classes('text-base text-gray-500 mt-0 italic text-center')
                    with ui.row().classes('items-center w-full'):
                        ui.label('Last response time: ').classes('text-base text-gray-500 mt-0 italic text-center')
                        last_response[dev_idx] = ui.label('never').classes('text-base text-gray-500 mt-0 italic text-center')
                    
                    # Configuration - deux champs
                    ui.separator()
                    ui.label('SEQUENCE').classes('text-3xl text-gray-600 mt-6 mb-2')

                    # Champ ENVOI
                    with ui.row().classes('items-center gap-3 w-full'):
                        send_inp = ui.input(
                            placeholder='ex: 01 02 A3 FF',
                            label='Sequence à envoyer'
                        ).classes('w-full font-mono text-lg bg-zinc-950').props('outlined clearable')

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
                        if send_inp.value is None:
                            return
                        formatted = format_hex(send_inp.value)
                        if formatted != send_inp.value:
                            send_inp.value = formatted
                        valid = is_valid_hex(send_inp.value)
                        send_inp.classes(
                            remove='border-red-500 border-green-500'
                        )

                        if send_inp.value:
                            send_inp.classes(
                                add='border-green-500' if valid
                                else 'border-red-500'
                            )

                    send_inp.on('update:model-value', on_send_change)
                    send_inputs[dev_idx] = send_inp
                    
                    # Champ LECTURE - Version améliorée pour 55 registres
                    ui.label('Sequence lue').classes('text-lg text-gray-400 mt-6 mb-2')
                    read_out = ui.textarea(
                        label='',
                        placeholder='Les données hex apparaîtront ici...'
                    ).classes('w-full font-mono text-lg bg-zinc-950').props('outlined readonly')   

                    read_outputs[dev_idx] = read_out

                    status_lbl = ui.label('').classes('text-xs mt-1 min-h-5')
                    status_labels[dev_idx] = status_lbl

                    # Boutons
                    with ui.row().classes('gap-3 mt-4 w-full'):
                        async def send_action(dev_id=dev_idx):
                            # Protection renforcée
                            if dev_id not in send_inputs or send_inputs[dev_id].value is None:
                                ui.notify('Champ vide ou invalide', type='warning')
                                return
                            val = send_inputs[dev_id].value.strip()
                            if not val:
                                ui.notify('Rien à envoyer', type='warning')
                                return
                            # Feedback immédiat
                            status_labels[dev_id].text = 'Envoi en cours...'
                            status_labels[dev_id].classes(replace='text-amber-600')
                            
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
    
    # =========================================================
    # BATTERIES
    # =========================================================
    battery_state_labels = {}
    battery_pack_labels = {}
    battery_cell_labels = {}

    battery_tabs_bar = {}

    # =========================================================
    # ONGLETS BATTERIES
    # =========================================================

    with ui.tabs().classes('w-full text-lg mt-8') as battery_tabs_bar:
        battery_tab_list = []
        for sid in BATTERY_IDS:
            tab = ui.tab(f'PILE {sid}') \
                .classes(
                    'rounded-xl shadow font-bold transition-all duration-300'
                )
            battery_tabs_dict[sid] = tab
            battery_tab_states[sid] = 'UNKNOWN'
            battery_tab_list.append(tab)
    
    # =========================================================
    # PANELS BATTERIES
    # =========================================================
    with ui.tab_panels(battery_tabs_bar, value=battery_tab_list[0]).classes('w-full shadow-lg rounded-xl'):
        last_request_batt = {}
        last_response_batt = {}

        for sid, tab in zip(BATTERY_IDS, battery_tab_list):
            with ui.tab_panel(tab):
                with ui.card().classes('w-full'):
                    ui.label(f'Battery {sid}').classes('text-4xl font-bold mb-4')

                    # =====================================================
                    # ETAT GLOBAL
                    # =====================================================

                    with ui.row().classes('items-center gap-4 mb-4'):
                        ui.label('État :').classes('text-2xl')
                        lbl = ui.label('---').classes('text-3xl font-bold')
                        battery_state_labels[sid] = lbl

                    # =====================================================
                    # TENSION PACK
                    # =====================================================

                    with ui.row().classes('items-center gap-4 mb-6'):
                        ui.label('Tension pack :').classes('text-2xl')
                        lbl = ui.label('--- mV').classes('text-3xl font-bold')
                        battery_pack_labels[sid] = lbl

                    ui.separator()

                    # =====================================================
                    # TENSIONS CELLULES
                    # =====================================================

                    ui.label('Tensions cellules').classes('text-2xl mt-6 mb-4')

                    cell_labels = []

                    with ui.grid(columns=4).classes('w-full gap-4'):

                        for label in LIST_BAT_CELLS.values():
                            with ui.card().classes('items-center bg-zinc-800 p-3 rounded-xl'):
                                ui.label(label).classes('text-white text-lg')
                                lbl = ui.label('--- mV').classes('text-2xl font-mono')
                                cell_labels.append(lbl)
                        # for i in range(NUM_BAT_CELLS):
                        #     with ui.card().classes('items-center bg-zinc-800 p-3 rounded-xl'):
                        #         if i == 0:
                        #             ui.label(f'CELL 1 & 2').classes('text-white text-lg')
                        #         elif i < 3:
                        #             ui.label(f'CELL {i + 2}').classes('text-white text-lg')
                        #         elif i == 3:
                        #             ui.label(f'CELL 5 & 6').classes('text-white text-lg')
                        #         else:
                        #             ui.label(f'CELL {i + 3}').classes('text-white text-lg')
                        #         lbl = ui.label('--- mV').classes('text-2xl font-mono')
                        #         cell_labels.append(lbl)

                    battery_cell_labels[sid] = cell_labels

                    with ui.row().classes('items-center w-full'):
                        ui.label('Last request time: ').classes('text-base text-gray-500 mt-0 italic text-center')
                        last_request_batt[sid] = ui.label('never').classes('text-base text-gray-500 mt-0 italic text-center')
                    with ui.row().classes('items-center w-full'):
                        ui.label('Last response time: ').classes('text-base text-gray-500 mt-0 italic text-center')
                        last_response_batt[sid] = ui.label('never').classes('text-base text-gray-500 mt-0 italic text-center')
    
    with ui.card().classes('w-full mt-6'):
        ui.label(
            'Historique des cellules'
        ).classes(
            'text-2xl font-bold'
        )

        with ui.row().classes('gap-4 items-center'):

            selected_battery = ui.select(
                BATTERY_IDS,
                value=BATTERY_IDS[0],
                label='Pile'
            ).classes('w-40')

            selected_cell = ui.select(
                options=LIST_BAT_CELLS,
                value=0,
                label='Cellule'
            ).classes('w-40')                
        
        chart = ui.echart({
            'title': {'text': ''},
            'tooltip': {'trigger': 'axis'},
            'xAxis': {
                'type': 'category',
                'data': [],
            },
            'yAxis': {
                'type': 'value',
                'name': 'mV',
                'min': 2000,
                'max': 4500,
            },
            'visualMap': {
                'show': False,
                'dimension': 1,
                'pieces': [
                    { 'lte': 2500, 'color': '#dc2626'},
                    { 'gt': 2500, 'lte': 3000, 'color': '#f97316'},
                    { 'gt': 3000, 'lte': 3300, 'color': '#eab308'},
                    { 'gt': 3300, 'color': '#22c55e'}
                ]
            },
            'series': [{
                'name': 'Tension',
                'type': 'line',
                'smooth': True,
                'data': [],
                'markLine': {
                    'silent': True,
                    'data': [
                        {
                            'yAxis': 2500,
                            'lineStyle': {
                                'color': '#dc2626',
                                'width': 2,
                                'type': 'dashed'
                            },
                            'label': {'formatter': 'Seuil critique', 'color': '#ffffff'}
                        },
                        {   
                            'yAxis': 3000,
                            'lineStyle': {
                                'color': '#f97316',
                                'width': 2,
                                'type': 'dashed'
                            },
                            'label': {'formatter': 'Seuil bas', 'color': '#ffffff'}
                        },
                        {   
                            'yAxis': 3300,
                            'lineStyle': {
                                'color': '#22c55e',
                                'width': 2,
                                'type': 'dashed'
                            },
                            'label': {'formatter': 'Seuil moyen', 'color': '#ffffff'}
                        }
                    ]
                }
            }],
        }).classes('w-full h-128')

    ui.separator()
    ui.label("Accès smartphone :")
    ui.label(url).classes("text-blue-600")
    
    async def refresh_ui():
        for sid in SLAVE_IDS:
            if sid not in active_slaves:
                if memo_state[sid] == False:
                    memo_state[sid] = True
                    memo_labels[sid] = global_labels[sid].text

                global_labels[sid].text = memo_labels[sid] + " (not refreshed)"
                global_labels[sid].classes(replace='text-3xl text-gray-600')
                continue
            else:
                memo_state[sid] = False

            state = device_states[sid]
            if not state["dirty"]:
                continue

            state["dirty"] = False
            txt, cls = state["last_request"]
            last_request[sid].text = txt
            last_request[sid].classes(replace=cls)

            txt, cls = state["last_response"]
            last_response[sid].text = txt
            last_response[sid].classes(replace=cls)
            
            if state["disconnected"]:
 
                global_labels[sid].text = "DISCONNECTED"
                global_labels[sid].classes(replace='text-3xl text-red-600 animate-pulse')
                continue 
 
            txt, cls = state["global"] 
 
            global_labels[sid].text = txt
            global_labels[sid].classes(replace=cls)

            update_pyro_tab_color(sid, txt)

            for i, (txt, cls) in enumerate(state["analogs"]):

                analog_labels[sid][i].text = txt
                analog_labels[sid][i].classes(replace=cls)
 
            alim_labels[sid].text = state["alim"]
            alim_1A_labels[sid].text = state["alim_1A"]

        for sid in BATTERY_IDS:
            # if sid not in active_slaves:
            #     if memo_state[sid] == False:
            #         memo_state[sid] = True
            #         memo_labels[sid] = global_labels[sid].text

            #     global_labels[sid].text = memo_labels[sid] + " (not refreshed)"
            #     global_labels[sid].classes(replace='text-3xl text-gray-600')
            #     continue
            # else:
            #     memo_state[sid] = False
            now = time.strftime('%H:%M:%S')
            
            state = battery_states[sid]
            if not state["dirty"]:
                    continue

            state["dirty"] = False
            
            txt, cls = state["last_request"]
            last_request_batt[sid].text = txt
            last_request_batt[sid].classes(replace=cls)

            txt, cls = state["last_response"]
            last_response_batt[sid].text = txt
            last_response_batt[sid].classes(replace=cls)
            
            if state["disconnected"]:

                battery_state_labels[sid].text = "DISCONNECTED"
                battery_state_labels[sid].classes(replace='text-3xl text-red-600 animate-pulse')
                continue

            txt, cls = state["global"]

            battery_state_labels[sid].text = txt
            battery_state_labels[sid].classes(replace=cls)

            update_battery_tab_color(sid, txt)

            battery_pack_labels[sid].text = f'{state["pack"]} mV'

            for i, txt in enumerate(state["cell"]):
                if txt != '—':
                    if i == 0 or i == 3:
                        val = int(txt) / 2
                        txt = str(int(val))
                        
                    if int(txt) < 2500:
                        color = 'text-red-600 font-bold'
                    elif int(txt) < 3000:
                        color = 'text-orange-500 font-bold'
                    elif int(txt) < 3300:
                        color = 'text-yellow-400'
                    else:
                        color = 'text-green-600'

                    battery_cell_labels[sid][i].text = f'{txt} mV'
                    cell_history[sid][i].append((now, int(txt)))
                else:
                    battery_cell_labels[sid][i].text = f'{txt} mV'
                    color = ''
                battery_cell_labels[sid][i].classes(replace=f'text-2xl font-mono {color}')

    ui.timer(0.5, refresh_ui)
    
    def update_chart():
        sid = selected_battery.value
        cell_idx = selected_cell.value

        if sid is None or cell_idx is None:
            print("error1")
            return
        if sid not in cell_history:
            print("error2")
            return
    
        history = list(cell_history[sid][cell_idx])

        chart.options['xAxis']['data'] = [
            t for t, _ in history
        ]

        chart.options['series'][0]['data'] = [
            {
                'value': v,
                'itemStyle': {
                    'color': (
                        '#dc2626' if v < 2500 else   # rouge
                        '#f97316' if v < 3000 else   # orange
                        '#22c55e' if v >= 3300 else  # vert
                        '#eab308'                    # jaune 3000-3299
                    )
                }
            }
            for _, v in history
        ]

        chart.options['title']['text'] = (
            # f'Pile {sid} - {selected_cell.value}'
            f'Pile {sid} - {LIST_BAT_CELLS[cell_idx]}'
        )
       

        chart.update()

    selected_battery.on_value_change(update_chart)
    selected_cell.on_value_change(update_chart)

    ui.timer(1.0, update_chart)

    async def close_app():
        global modbus_running
        modbus_running = False
        os._exit(0)

    app.on_disconnect(close_app)


def main():  
    threading.Thread(
        target=modbus_worker,
        daemon=True
    ).start()

    ui.run(
        title='MGPF',
        favicon='asset/icon.ico',
        host='0.0.0.0',
        port=8080,
        dark=True,
        reload=False,
        show=True,
        access_log=False
    )

#if __name__ in {"__main__", "__mp_main__"}:
if __name__ == "__main__":
    multiprocessing.freeze_support()
    main()
