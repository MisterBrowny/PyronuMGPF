import flet as ft
from flet import icons
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
TIMEOUT = 1.0

NUM_DEVICES = 20
NUM_ANALOG_INPUTS = 16

GLOBAL_STATE_REG = 100
ANALOG_START_REG = 101
CONFIG_START_REG = 200
CONFIG_NUM_REGS = 8

SLAVE_IDS = list(range(1, NUM_DEVICES + 1))
REFRESH_INTERVAL = 5.0

GLOBAL_MAP = {
    0: ("END",    ft.Colors.GREY_700),
    1: ("GO",     ft.Colors.GREEN_600),
    2: ("ARMED",  ft.Colors.BLUE_600),
    3: ("TEST",   ft.Colors.ORANGE_600),
}

# ===================== VARIABLES =====================
selected_port = "/dev/ttyUSB0"
active_slaves = set(SLAVE_IDS)          # slaves cochés
module_containers = {}                  # dev → Container de la carte

def get_ports():
    return [p.device for p in serial.tools.list_ports.comports()] or ["Aucun port"]

# ===================== MODBUS =====================
def modbus_client():
    return ModbusClient(method='rtu', port=selected_port, baudrate=BAUDRATE,
                        parity=PARITY, stopbits=STOPBITS, bytesize=BYTESIZE, timeout=TIMEOUT)

def modbus_read(slave_id: int):
    client = modbus_client()
    try:
        if not client.connect():
            return None, None, "Connexion impossible"
        resp = client.read_holding_registers(GLOBAL_STATE_REG, 3, slave=slave_id)
        if resp.isError():
            return None, None, str(resp)

        regs = resp.registers
        global_val = regs[0] & 0xFF
        g_state = GLOBAL_MAP.get(global_val, ("???", ft.Colors.PURPLE_600))

        bits32 = (regs[2] << 16) | regs[1]
        states = []
        for i in range(16):
            val = (bits32 >> (i * 2)) & 0b11
            state = "OK" if val == 3 else "MOYEN" if val == 2 else "KO" if val == 1 else "ABSENT"
            states.append(state)
        return g_state, states, None
    except Exception as e:
        return None, None, str(e)
    finally:
        client.close()

# ===================== INTERFACE FLET =====================
def main(page: ft.Page):
    page.title = "MODBUS FIREWORKS CONTROL"
    page.theme_mode = ft.ThemeMode.DARK
    page.bgcolor = "#0a0f1c"
    page.padding = 0
    page.window_width = 1400
    page.window_height = 900

    # ===================== HEADER =====================
    header = ft.Container(
        content=ft.Row([
            ft.Row([
                ft.Icon(ft.icons.Icons.ROCKET_LAUNCH_ROUNDED, color="#22d3ee", size=42),
                ft.Text("MODBUS FIREWORKS CONTROL", size=28, weight="bold", color="white")
            ]),
            ft.Row([
                ft.Container(ft.Text("ARMED", color="orange", weight="bold", size=16),
                            bgcolor="#1e2937", padding=8, border_radius=8),
                ft.Container(ft.Text("ONLINE", color="#22c55e", weight="bold", size=16),
                            bgcolor="#1e2937", padding=8, border_radius=8),
            ])
        ], alignment=ft.MainAxisAlignment.SPACE_BETWEEN),
        bgcolor="#0f172a",
        padding=15,
        border=ft.border.only(bottom=ft.border.BorderSide(2, "#334155"))
    )

    # ===================== PORT + APPLIQUER =====================
    port_select = ft.Dropdown(
        options=[ft.dropdown.Option(p) for p in get_ports()],
        value=selected_port,
        width=200,
        label="Port COM",
        bgcolor="#1e2937",
        border_color="#64748b"
    )

    def apply_selection(e):
        global active_slaves
        active_slaves = {SLAVE_IDS[i] for i, chk in enumerate(checkboxes) if chk.value}
        create_modules()
        log("Sélection appliquée – modules régénérés")

    apply_btn = ft.ElevatedButton("Appliquer sélection", color="white", bgcolor="#22d3ee", on_click=apply_selection)

    top_bar = ft.Row([port_select, apply_btn], alignment=ft.MainAxisAlignment.CENTER, spacing=30)

    # ===================== CHECKBOXES SLAVES =====================
    checkboxes = []
    check_row = ft.Row(wrap=True, spacing=15, run_spacing=10)
    for dev in range(1, NUM_DEVICES + 1):
        chk = ft.Checkbox(label=f"{dev:02d} (ID {SLAVE_IDS[dev-1]})", value=True, width=140)
        checkboxes.append(chk)
        check_row.controls.append(chk)

    # ===================== MODULES GRID (2x8 LEDs) =====================
    modules_grid = ft.GridView(expand=True, runs_count=4, spacing=15, run_spacing=15, padding=20)

    def create_modules():
        modules_grid.controls.clear()
        module_containers.clear()

        for dev in range(1, NUM_DEVICES + 1):
            sid = SLAVE_IDS[dev - 1]
            if sid not in active_slaves:
                continue

            # LEDs 2x8 avec ordre demandé (ligne impaire = 1,3,5… / ligne paire = 2,4,6…)
            led_grid = ft.GridView(runs_count=2, spacing=4, run_spacing=4, width=220, height=70)
            leds = []
            for row in range(2):           # 0 = ligne impaire, 1 = ligne paire
                for col in range(8):
                    idx = row * 8 + col
                    logical_idx = (col * 2) + row      # 0,2,4... puis 1,3,5...
                    led = ft.Container(width=18, height=18, border_radius=4, bgcolor="#334155")
                    leds.append(led)
                    led_grid.controls.append(led)

            # Carte
            card = ft.Card(
                content=ft.Container(
                    content=ft.Column([
                        ft.Text(f"Module {dev:02d}", size=16, weight="bold", color="#67e8f9", text_align=ft.TextAlign.CENTER),
                        led_grid,
                        ft.Row([
                            ft.Container(ft.Text("READY", size=12, weight="bold", color="white"),
                                        bgcolor="#22c55e", padding=6, border_radius=6),
                            ft.Container(ft.Text("ERROR", size=12, weight="bold", color="white"),
                                        bgcolor="#ef4444", padding=6, border_radius=6)
                        ], alignment=ft.MainAxisAlignment.CENTER),
                        ft.Container(ft.Text("—", size=14, weight="bold"),
                                    bgcolor="#1e2937", padding=8, border_radius=8, width=120)
                    ], spacing=12, alignment=ft.MainAxisAlignment.CENTER),
                    padding=16,
                    bgcolor="#1e2937",
                    border_radius=12
                ),
                elevation=8
            )

            modules_grid.controls.append(card)
            module_containers[dev] = (leds, card)   # leds[0..15] + card

    # ===================== EVENT LOG =====================
    log_list = ft.ListView(expand=True, spacing=6, padding=10, auto_scroll=True)

    def log(message: str):
        log_list.controls.append(
            ft.Text(f"[{time.strftime('%H:%M:%S')}] {message}", color="#67e8f9", size=13)
        )
        if len(log_list.controls) > 30:
            log_list.controls.pop(0)
        log_list.update()

    # ===================== LAYOUT FINAL =====================
    page.add(
        header,
        ft.Container(top_bar, padding=10),
        ft.Container(check_row, padding=10),
        ft.Container(modules_grid, expand=True, padding=10),
        ft.Container(
            content=ft.Column([
                ft.Text("Event Log", size=18, weight="bold", color="#67e8f9"),
                log_list
            ]),
            bgcolor="#0f172a",
            border=ft.border.all(1, "#334155"),
            padding=10,
            height=220
        )
    )

    # Lancement
    create_modules()
    log("Application démarrée")

    # Rafraîchissement auto
    async def auto_refresh():
        while True:
            await refresh_modules()
            await asyncio.sleep(REFRESH_INTERVAL)

    asyncio.create_task(auto_refresh())
    page.update()


# ===================== RAFRAÎCHISSEMENT =====================
async def refresh_modules():
    for dev in module_containers:
        sid = SLAVE_IDS[dev - 1]
        if sid not in active_slaves:
            continue

        g_state, analog_states, err = await asyncio.to_thread(modbus_read, sid)
        if err:
            continue

        leds, card = module_containers[dev]
        # Mise à jour LEDs (ordre 2x8 avec ligne impaire/paire)
        for i, state in enumerate(analog_states):
            color = {
                "OK": "#22c55e", "MOYEN": "#eab308",
                "KO": "#ef4444", "ABSENT": "#475569"
            }.get(state, "#64748b")
            leds[i].bgcolor = color

        # Mise à jour état global (à améliorer si tu veux un badge par carte)
        # Pour l'instant on met juste dans le log
    # log("Rafraîchissement terminé")

ft.app(target=main)