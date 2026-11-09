"""
conftest.py
Outils communs a tous les tests Python (pytest le lit automatiquement).

Le principe :
  1. on demarre le vrai programme BCM (./build/bcm)
  2. Python joue le TABLEAU DE BORD : il envoie DASH_Inputs toutes les 100 ms
  3. Python ECOUTE les messages BCM_Lamps et BCM_Status et verifie les regles
"""
import os
import subprocess
import threading
import time
from pathlib import Path

import can
import cantools
import pytest

# Chemins du projet
PROJECT = Path(__file__).resolve().parents[2]
DBC_FILE = PROJECT / "can" / "bcm.dbc"
BCM_PROGRAM = Path(os.environ.get("BCM_BIN", PROJECT / "build" / "bcm"))


class BcmTester:
    """Le 'conducteur' virtuel : il appuie sur les boutons et regarde les lampes."""

    def __init__(self, bus, db):
        self.bus = bus
        self.db = db
        # Ce que le tableau de bord envoie en ce moment (contact mis par defaut)
        self.dash = {"Lever": 0, "Hazard": 0, "KeyLock": 0, "KeyUnlock": 0, "Ignition": 1}
        self.dash_running = True
        self._stop = False
        self._thread = threading.Thread(target=self._dashboard_loop, daemon=True)
        self._thread.start()

    # ---------- Le tableau de bord (envoie DASH_Inputs toutes les 100 ms) ----------
    def _dashboard_loop(self):
        while not self._stop:
            if self.dash_running:
                self.send("DASH_Inputs", **self.dash)
            time.sleep(0.1)

    def set_dashboard(self, **signals):
        """Exemple : set_dashboard(Lever=1) = mettre le levier a gauche."""
        self.dash.update(signals)

    def stop_dashboard(self):
        """Simule une panne : le tableau de bord ne parle plus."""
        self.dash_running = False

    # ---------- Envoyer un message decrit dans le DBC ----------
    def send(self, message_name, **signals):
        msg = self.db.get_message_by_name(message_name)
        data = msg.encode(signals)
        self.bus.send(can.Message(arbitration_id=msg.frame_id, data=data, is_extended_id=False))

    # ---------- Ecouter les messages du BCM ----------
    def record(self, message_name, duration):
        """Enregistre pendant 'duration' secondes. Retourne une liste de (heure, signaux)."""
        frame_id = self.db.get_message_by_name(message_name).frame_id
        result = []
        end = time.monotonic() + duration
        while time.monotonic() < end:
            msg = self.bus.recv(timeout=0.05)
            if msg is not None and msg.arbitration_id == frame_id:
                result.append((time.monotonic(), self.db.decode_message(frame_id, msg.data, decode_choices=False)))
        return result

    def latest(self, message_name, duration=0.1):
        """Le dernier message recu (on ecoute un court instant)."""
        frames = self.record(message_name, duration)
        assert frames, f"aucun message {message_name} recu"
        return frames[-1][1]

    def wait_for(self, message_name, condition, timeout):
        """Attend qu'un message respecte la condition. Retourne True si c'est arrive a temps."""
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            for _, signals in self.record(message_name, 0.05):
                if condition(signals):
                    return True
        return False

    def close(self):
        self._stop = True
        self._thread.join(timeout=1)


# ---------- Outil de mesure : le rythme d'un clignotant ----------
def blink_half_periods(frames, signal):
    """Retourne les durees (en ms) entre deux changements d'etat d'une lampe."""
    changes = []
    previous = None
    for t, signals in frames:
        if previous is not None and signals[signal] != previous:
            changes.append(t)
        previous = signals[signal]
    return [round((b - a) * 1000) for a, b in zip(changes, changes[1:])]


# ---------- Fixture : un BCM tout neuf pour chaque test ----------
@pytest.fixture
def bcm():
    if not BCM_PROGRAM.exists():
        pytest.fail(f"Programme introuvable : {BCM_PROGRAM}. Lance d'abord : cmake --build build")

    db = cantools.database.load_file(str(DBC_FILE))
    try:
        bus = can.Bus(interface="socketcan", channel="vcan0")
    except OSError:
        pytest.fail("vcan0 introuvable. Lance d'abord : ./scripts/setup_vcan.sh")

    process = subprocess.Popen([str(BCM_PROGRAM)], stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, text=True)
    tester = BcmTester(bus, db)
    time.sleep(0.5)                  # laisser le BCM demarrer
    yield tester                     # <- le test s'execute ici

    tester.close()                   # nettoyage apres le test
    process.terminate()
    process.wait(timeout=2)
    bus.shutdown()
