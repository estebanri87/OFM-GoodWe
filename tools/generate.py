# -*- coding: utf-8 -*-
"""Erzeugt alle generierten Quellen des Moduls. Nach jeder Aenderung an slots.py oder
script.template.js aufrufen, danach die knxprod neu bauen.

    python tools/generate.py

Erzeugt:
    src/GoodWe.templ.xml   Kanalseite mit allen Objekten
    src/SlotCatalog.h/.cpp derselbe Katalog fuer die Firmware
    src/GoodWe.script.js   ETS-Skript des Assistenten
"""
import os
import runpy
import sys

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, here)

runpy.run_path(os.path.join(here, "gen_templ.py"), run_name="__main__")
