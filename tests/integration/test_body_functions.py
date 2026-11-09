"""
test_body_functions.py
Scenarios "conducteur" pour l'ampoule grillee, les portes, les phares et l'alarme.
"""
import time

from conftest import blink_half_periods

# Valeurs de AlarmStatus (voir VAL_ dans can/bcm.dbc)
DISARMED, ARMING, ARMED, TRIGGERED = 0, 1, 2, 3


def press(bcm, key):
    """Appuyer sur un bouton de la cle puis le relacher (KeyLock ou KeyUnlock)."""
    bcm.set_dashboard(**{key: 1})
    time.sleep(0.3)
    bcm.set_dashboard(**{key: 0})
    time.sleep(0.2)


def park(bcm):
    """Couper le contact, puis attendre que le BCM l'ait bien pris en compte."""
    bcm.set_dashboard(Ignition=0)
    time.sleep(0.3)


# ------------------------- Ampoule grillee -------------------------
def test_bulb_failure_blinks_twice_as_fast(bcm):
    bcm.set_dashboard(Lever=1)
    bcm.send("LAMP_Diag", BulbLeftFailed=1, BulbRightFailed=0)
    frames = bcm.record("BCM_Lamps", duration=2.0)

    half_periods = blink_half_periods(frames, "LampLeft")
    assert len(half_periods) >= 6, "pas assez de clignotements mesures"
    for duration in half_periods:
        assert 120 <= duration <= 230, f"rythme incorrect : {duration} ms au lieu de ~167 ms"


# ------------------------- Portes -------------------------
def test_doors_lock_automatically_above_15_kmh(bcm):
    bcm.send("VEH_Speed", Speed=10)
    time.sleep(0.3)
    assert bcm.latest("BCM_Status")["DoorsLocked"] == 0, "a 10 km/h, pas de verrouillage"

    bcm.send("VEH_Speed", Speed=30)
    assert bcm.wait_for("BCM_Status", lambda s: s["DoorsLocked"] == 1, timeout=1.0), \
        "a 30 km/h, les portes doivent se verrouiller"


def test_key_buttons_lock_and_unlock(bcm):
    press(bcm, "KeyLock")
    assert bcm.wait_for("BCM_Status", lambda s: s["DoorsLocked"] == 1, timeout=1.0)
    press(bcm, "KeyUnlock")
    assert bcm.wait_for("BCM_Status", lambda s: s["DoorsLocked"] == 0, timeout=1.0)


# ------------------------- Phares automatiques -------------------------
def test_headlights_turn_on_in_tunnel(bcm):
    bcm.send("ENV_Light", Light=200)          # entree dans un tunnel
    time.sleep(1.0)
    assert bcm.latest("BCM_Lamps")["Headlights"] == 0, "trop tot : on attend 2 s (temporisation)"

    assert bcm.wait_for("BCM_Lamps", lambda s: s["Headlights"] == 1, timeout=2.5), \
        "apres 2 s dans le noir, les phares doivent s'allumer"


def test_short_bridge_does_not_turn_headlights_on(bcm):
    bcm.send("ENV_Light", Light=200)          # sous un pont...
    time.sleep(1.0)
    bcm.send("ENV_Light", Light=10000)        # ... et deja ressorti
    time.sleep(2.0)
    assert bcm.latest("BCM_Lamps")["Headlights"] == 0, "un pont de 1 s ne doit rien allumer"


# ------------------------- Alarme -------------------------
def test_alarm_arming_and_disarming(bcm):
    park(bcm)
    press(bcm, "KeyLock")
    assert bcm.wait_for("BCM_Status", lambda s: s["AlarmStatus"] == ARMING, timeout=1.0)

    press(bcm, "KeyUnlock")
    assert bcm.wait_for("BCM_Status", lambda s: s["AlarmStatus"] == DISARMED, timeout=1.0)


def test_thief_opens_door_triggers_alarm(bcm):
    """Scenario complet (~25 s) : garer, verrouiller, attendre 20 s, ouvrir une porte."""
    park(bcm)
    press(bcm, "KeyLock")
    assert bcm.wait_for("BCM_Status", lambda s: s["AlarmStatus"] == ARMED, timeout=25.0), \
        "l'alarme doit etre armee 20 s apres le verrouillage"

    bcm.send("DOOR_Status", DoorOpen=1)       # un voleur ouvre la porte
    assert bcm.wait_for("BCM_Lamps", lambda s: s["Siren"] == 1, timeout=1.0), "la sirene doit sonner"

    frames = bcm.record("BCM_Lamps", duration=1.5)
    assert any(s["LampLeft"] == 1 and s["LampRight"] == 1 for _, s in frames), \
        "les deux clignotants doivent flasher"
