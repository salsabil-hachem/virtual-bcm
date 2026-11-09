"""
test_turn_signals.py
Scenarios "conducteur" pour les clignotants, sur le vrai BCM (FreeRTOS + CAN).
"""
from conftest import blink_half_periods


def test_left_lever_blinks_left_lamp_only(bcm):
    bcm.set_dashboard(Lever=1)                       # levier a gauche
    frames = bcm.record("BCM_Lamps", duration=2.0)   # on regarde 2 secondes

    left = [s["LampLeft"] for _, s in frames]
    right = [s["LampRight"] for _, s in frames]
    assert 1 in left and 0 in left, "la lampe gauche doit s'allumer ET s'eteindre"
    assert 1 not in right, "la lampe droite doit rester eteinte"


def test_blink_rhythm_is_333_ms(bcm):
    bcm.set_dashboard(Lever=1)
    frames = bcm.record("BCM_Lamps", duration=3.0)

    half_periods = blink_half_periods(frames, "LampLeft")
    assert len(half_periods) >= 4, "pas assez de clignotements mesures"
    for duration in half_periods:
        assert 250 <= duration <= 420, f"rythme incorrect : {duration} ms au lieu de ~333 ms"


def test_hazard_wins_over_lever(bcm):
    bcm.set_dashboard(Lever=2, Hazard=1)             # levier a droite + warning
    frames = bcm.record("BCM_Lamps", duration=2.0)

    both_on = [s for _, s in frames if s["LampLeft"] == 1 and s["LampRight"] == 1]
    only_one = [s for _, s in frames if s["LampLeft"] != s["LampRight"]]
    assert both_on, "les deux lampes doivent s'allumer ensemble"
    assert not only_one, "les deux lampes doivent toujours etre synchronisees"


def test_safe_mode_when_dashboard_silent(bcm):
    assert bcm.latest("BCM_Status")["Faults"] == 0, "pas de defaut au depart"

    bcm.stop_dashboard()                             # panne du tableau de bord
    assert bcm.wait_for("BCM_Status", lambda s: s["Faults"] == 1, timeout=2.0), \
        "le BCM doit signaler la perte du tableau de bord"
    assert bcm.latest("BCM_Lamps")["Headlights"] == 1, "mode securite : phares allumes"
