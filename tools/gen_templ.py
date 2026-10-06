# -*- coding: utf-8 -*-
"""Erzeugt src/GoodWe.templ.xml, src/SlotCatalog.h/.cpp und src/GoodWe.script.js aus tools/slots.py.

Ein Kanal ist ein Wechselrichter. Jedes Objekt des Katalogs hat ein eigenes Aktiv-Haekchen;
der Assistent "Wechselrichter auslesen" setzt sie anhand dessen, was das Geraet liefert.

Aufruf:  python tools/gen_templ.py
"""
import io
import os

from slots import (COND, CALCS, DIVS, ENABLE_BITS, SENSOR_CAPACITY, SENSOR_GROUPS, SENSORS,
                   SETTING_CAPACITY, SETTING_KO_BASE, SETTINGS, TYPES, cond_mask, setting_ko)

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "..", "src")

# ---------------------------------------------------------------- Speicherlayout (Byte)
TYPE_OFFSET = 0
SUSPENDED_OFFSET = 1
IP_OFFSET = 2            # 16 Byte IPv4-Literal
PORT_OFFSET = 18
PROTOCOL_OFFSET = 20
ADDRESS_OFFSET = 21
INTERVAL_OFFSET = 22
SEND_OFFSET = 24         # Zeitbasis + Zeit (16 Bit), Aenderung in % (8 Bit)
ENABLE_OFFSET = 32       # Aktiv-Bits, 64 Byte Platz
ENABLE_BYTES = 64
BLOCK_BYTES = ENABLE_OFFSET + ENABLE_BYTES
assert ENABLE_BITS <= ENABLE_BYTES * 8

# Parameternummern
ENABLE_PARAM_BASE = 100  # Aktiv-Bit k -> UP-...(100 + k)

P = "%"
CH = P + "C" + P
PS = P + "AID" + P + "_PS-nnn"


def uid(n):
    return "%sAID%s_UP-%sTT%s%sCC%s%03d" % (P, P, P, P, P, P, n)


def uref(n):
    return uid(n) + "_R-%sTT%s%sCC%s%03d01" % (P, P, P, P, n)


def pid(n):
    return "%sAID%s_P-%sTT%s%sCC%s%03d" % (P, P, P, P, P, P, n)


def pref(n):
    return pid(n) + "_R-%sTT%s%sCC%s%03d01" % (P, P, P, P, n)


def oid(n):
    return "%sAID%s_O-%sTT%s%sCC%s%03d" % (P, P, P, P, P, P, n)


def oref(n):
    return oid(n) + "_R-%sTT%s%sCC%s%03d01" % (P, P, P, P, n)


NAMEREF = pref(0)
DISPLAYREF = pref(998)
TYPEREF = uref(10)
TYPESELREF = pref(11)
SUSPREF = uref(12)
INFOREF = pref(13)

# Familien -> Typwerte des Haupttyps (1 = automatisch, 2 = Hybrid, 3 = netzgekoppelt)
FAM_TEST = {"C": None, "E": "1 2", "D": "1 3"}
FAM_MASK = {"C": 3, "E": 1, "D": 2}


def xml(s):
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace('"', "&quot;")


def function_text(dpt, unit, label):
    """Objektfunktion nach dem Muster '<Modul> %C%: Ausgang, <Groesse> (<Einheit>)'."""
    d = dpt[0]
    if d == "DPST-14-56":
        if unit == "var":
            return "Blindleistung (var)"
        if unit == "VA":
            return "Scheinleistung (VA)"
        return "Leistung (W)"
    if d == "DPST-14-27":
        return "Spannung (V)"
    if d == "DPST-14-19":
        return "Strom (A)"
    if d == "DPST-14-33":
        return "Frequenz (Hz)"
    if d == "DPST-14-57":
        return "Leistungsfaktor"
    if d == "DPST-13-10":
        return "Energie (Wh)"
    if d == "DPST-9-1":
        return "Temperatur (°C)"
    if d == "DPST-5-1":
        return "Prozent (%)"
    if d == "DPST-5-10":
        return "Code"
    if d == "DPST-7-1":
        return "Kapazität (Ah)" if unit == "Ah" else "Wert"
    if d == "DPST-12-1":
        return "Stunden (h)" if unit == "h" else "Bitfeld"
    if d == "DPST-19-1":
        return "Datum/Uhrzeit"
    if d == "DPST-1-11":
        return "Status"
    if d == "DPST-1-1":
        return "Ein/Aus"
    if d == "DPST-1-17":
        return "Auslöser"
    raise ValueError(d)


def with_unit(label, unit):
    """Anzeigetext der Zeile: Einheit anhaengen, wenn sie nicht schon im Text steht."""
    return label + (" (%s)" % unit if unit and "(" not in label else "")


# Alle Aktiv-Bits: (Bitnummer, Name, Anzeigetext, Familie). Der Anzeigetext steht im
# Parameter selbst - jedes Objekt ist eine normale ETS-Zeile mit Text und Haekchen.
BITS = []
for k, s in enumerate(SENSORS):
    BITS.append((k, s[0], with_unit(s[1], s[2]), s[4]))
for n, st in enumerate(SETTINGS):
    BITS.append((SENSOR_CAPACITY + n, st[0], st[1], st[2]))


def bit_param(k):
    return ENABLE_PARAM_BASE + k


L = []
A = L.append
A('<?xml version="1.0" encoding="utf-8"?>')
A('<?xml-model href="../../Organization/knxprod-support/knx_project_14/knx-editor.xsd" type="application/xml" schematypens="http://www.w3.org/2001/XMLSchema"?>')
A('<!-- ERZEUGT von tools/gen_templ.py aus tools/slots.py - nicht von Hand bearbeiten. -->')
A('<KNX xmlns:op="http://github.com/OpenKNX/OpenKNXproducer" xmlns="http://knx.org/xml/project/14" CreatedBy="KNX MT" ToolVersion="5.1.255.16695">')
A('  <ManufacturerData>')
A('    <Manufacturer>')
A('      <ApplicationPrograms>')
A('        <ApplicationProgram>')
A('          <Static>')
A('            <Parameters>')
A('              <!-- Ein Kanal = ein Wechselrichter. -->')
A('              <Parameter Id="%s" Name="CH%sName" ParameterType="%sAID%s_PT-Text40Byte" Text="Beschreibung" Value="" />' % (pid(0), CH, P, P))
A('              <!-- Anzeigename für den Kanal-Tab: Beschreibung, der bei "Suspendiert = Ja"')
A('                   das Zeichen der Kanalauswahl vorangestellt wird. Nur in der ETS. -->')
A('              <Parameter Id="%s" Name="CH%sNameDisplay" ParameterType="%sAID%s_PT-Text45Byte" Text="Beschreibung (Anzeige)" Value="" Access="Read" op:configTransfer="never" />' % (pid(998), CH, P, P))
A('              <!-- TypeSelect: kein Memory, nur in der ETS; Kanal-Tab, ohne Deaktiviert -->')
A('              <Parameter Id="%s" Name="CH%sTypeSelect" ParameterType="%sAID%s_PT-GDWTypeSelect" Text="Wechselrichtertyp" Value="1" />' % (pid(11), CH, P, P))
A('              <!-- Ergebnis von "Wechselrichter auslesen". Nur in der ETS. -->')
A('              <Parameter Id="%s" Name="CH%sDetected" ParameterType="%sAID%s_PT-GDWInfoText" Text="Erkannt" Value="" op:configTransfer="never" />' % (pid(13), CH, P, P))
A('              <!-- Haupttyp: Kanalauswahl-Tabelle, mit Deaktiviert -->')
A('              <Union SizeInBit="8">')
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, TYPE_OFFSET))
A('                <Parameter Id="%s" Name="CH%sType" ParameterType="%sAID%s_PT-GDWChannelType" Offset="0" BitOffset="0" Text="Wechselrichtertyp" Value="0" />' % (uid(10), CH, P, P))
A('              </Union>')
A('              <Union SizeInBit="8">')
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, SUSPENDED_OFFSET))
A('                <Parameter Id="%s" Name="CH%sSuspended" ParameterType="%sAID%s_PT-Suspended" Offset="0" BitOffset="0" Text="Suspendiert" Value="0" />' % (uid(12), CH, P, P))
A('              </Union>')
A('              <!-- Nur IPv4-Literale: eine Namensauflösung würde die Firmware blockieren. -->')
A('              <Union SizeInBit="128">')
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, IP_OFFSET))
A('                <Parameter Id="%s" Name="CH%sIp" ParameterType="%sAID%s_PT-GDWIp" Offset="0" BitOffset="0" Text="IP-Adresse" Value="" />' % (uid(1), CH, P, P))
A('              </Union>')
A('              <Union SizeInBit="16">')
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, PORT_OFFSET))
A('                <Parameter Id="%s" Name="CH%sPort" ParameterType="%sAID%s_PT-GDWPort" Offset="0" BitOffset="0" Text="Port" Value="8899" />' % (uid(2), CH, P, P))
A('              </Union>')
A('              <Union SizeInBit="8">')
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, PROTOCOL_OFFSET))
A('                <Parameter Id="%s" Name="CH%sProtocol" ParameterType="%sAID%s_PT-GDWProtocol" Offset="0" BitOffset="0" Text="Protokoll" Value="0" />' % (uid(3), CH, P, P))
A('              </Union>')
A('              <Union SizeInBit="8">')
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, ADDRESS_OFFSET))
A('                <Parameter Id="%s" Name="CH%sAddress" ParameterType="%sAID%s_PT-GDWAddress" Offset="0" BitOffset="0" Text="Modbus-Adresse" Value="247" />' % (uid(4), CH, P, P))
A('              </Union>')
A('              <Union SizeInBit="16">')
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, INTERVAL_OFFSET))
A('                <Parameter Id="%s" Name="CH%sPollInterval" ParameterType="%sAID%s_PT-GDWInterval" Offset="0" BitOffset="0" Text="Abfrageintervall" SuffixText=" s" Value="30" />' % (uid(5), CH, P, P))
A('              </Union>')
A('              <Union SizeInBit="16">')
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, SEND_OFFSET))
A('                <Parameter Id="%s" Name="CH%sSendDelayBase" ParameterType="%sAID%s_PT-DelayBase" Offset="0" BitOffset="0" Text="Zeitbasis" Value="2" />' % (uid(6), CH, P, P))
A('                <Parameter Id="%s" Name="CH%sSendDelayTime" ParameterType="%sAID%s_PT-DelayTime" Offset="0" BitOffset="2" Text="zyklisch senden alle (0 = nicht)" Value="0" />' % (uid(7), CH, P, P))
A('              </Union>')
A('              <Union SizeInBit="8">')
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, SEND_OFFSET + 2))
A('                <Parameter Id="%s" Name="CH%sSendChangePercent" ParameterType="%sAID%s_PT-GDWPercent" Offset="0" BitOffset="0" Text="bei Änderung um (0 = jede Änderung)" SuffixText=" %s" Value="5" />' % (uid(8), CH, P, P, P))
A('              </Union>')
A('              <!-- Aktiv-Bits, Byte %d-%d: Bit k = Objekt k aus tools/slots.py.' % (ENABLE_OFFSET, ENABLE_OFFSET + ENABLE_BYTES - 1))
A('                   Messwerte belegen Bit 0-%d, Einstellungen ab Bit %d.' % (SENSOR_CAPACITY - 1, SENSOR_CAPACITY))
A('                   BitOffset zählt innerhalb des Bytes vom höchstwertigen Bit (0) abwärts. -->')
A('              <Union SizeInBit="%d">' % (ENABLE_BYTES * 8))
A('                <Memory CodeSegment="%sMID%s" Offset="%d" BitOffset="0" />' % (P, P, ENABLE_OFFSET))
for k, name, label, fam in BITS:
    A('                <Parameter Id="%s" Name="CH%sUse%s" ParameterType="%sAID%s_PT-CheckBox" Offset="%d" BitOffset="%d" Text="%s" Value="0" />'
      % (uid(bit_param(k)), CH, name, P, P, k // 8, k % 8, xml(label)))
A('              </Union>')
A('            </Parameters>')
A('            <ParameterRefs>')
for n in (0, 998, 11, 13):
    A('              <ParameterRef Id="%s" RefId="%s" />' % (pref(n), pid(n)))
for n in (10, 12, 1, 2, 3, 4, 5, 6, 7, 8):
    A('              <ParameterRef Id="%s" RefId="%s" />' % (uref(n), uid(n)))
for k, _, _, _ in BITS:
    A('              <ParameterRef Id="%s" RefId="%s" />' % (uref(bit_param(k)), uid(bit_param(k))))
A('            </ParameterRefs>')
A('')
A('            <ParameterCalculations>')
A('              <!-- Haupttyp (Tabelle) und TypeSelect (Kanal-Tab) synchron halten -->')
A('              <ParameterCalculation Id="%sAID%s_PC-%sTT%s%sCC%s001" Language="JavaScript" Name="SyncType%sCC%s"' % (P, P, P, P, P, P, P, P))
A('                                    RLTransformationFunc="BASE_SyncChannelType"')
A('                                    LRTransformationFunc="BASE_SyncChannelType">')
A('                <LParameters>')
A('                  <ParameterRefRef RefId="%s" AliasName="TypeValue" />' % TYPESELREF)
A('                </LParameters>')
A('                <RParameters>')
A('                  <ParameterRefRef RefId="%s" AliasName="TypeValue" />' % TYPEREF)
A('                </RParameters>')
A('              </ParameterCalculation>')
A('              <!-- Suspendierte Kanäle in der Baumansicht kennzeichnen -->')
A('              <ParameterCalculation Id="%sAID%s_PC-%sTT%s%sCC%s002" Language="JavaScript" Name="MarkInactive%sCC%s"' % (P, P, P, P, P, P, P, P))
A('                                    RLTransformationFunc="BASE_MarkInactiveChannel" RLTransformationParameters="{&quot;InactiveValue&quot;:1}"')
A('                                    LRTransformationFunc="BASE_Nop">')
A('                <LParameters>')
A('                  <ParameterRefRef RefId="%s" AliasName="TextOutput" />' % DISPLAYREF)
A('                </LParameters>')
A('                <RParameters>')
A('                  <ParameterRefRef RefId="%s" AliasName="TextInput" />' % NAMEREF)
A('                  <ParameterRefRef RefId="%s" AliasName="InactiveControl" />' % SUSPREF)
A('                </RParameters>')
A('              </ParameterCalculation>')
A('            </ParameterCalculations>')

# ---------------------------------------------------------------- KOs
OUT = 'ReadFlag="Enabled" WriteFlag="Disabled" CommunicationFlag="Enabled" TransmitFlag="Enabled" UpdateFlag="Disabled" ReadOnInitFlag="Disabled"'
IN = 'ReadFlag="Disabled" WriteFlag="Enabled" CommunicationFlag="Enabled" TransmitFlag="Disabled" UpdateFlag="Enabled" ReadOnInitFlag="Disabled"'

KOS = []  # (Nummer, Name, Text, Objektfunktion, DPT, Groesse, Flags)
for k, s in enumerate(SENSORS):
    name, label, unit, dpt, fam, et, dt, cond = s
    KOS.append((k, name, label, "Ausgang, " + function_text(dpt, unit, label), dpt[0], dpt[1], OUT))
for n, st in enumerate(SETTINGS):
    name, label, fam, din, dout, desc = st
    koIn, koOut = setting_ko(n)
    ftIn = desc if desc and not desc in ("W", "%") else function_text(din, "", label)
    KOS.append((koIn, name, label, "Eingang, " + ftIn, din[0], din[1], IN))
    if dout is not None:
        ftOut = desc if desc and not desc in ("W", "%") else function_text(dout, "", label)
        KOS.append((koOut, name + "Status", "Status " + label, "Ausgang, " + ftOut, dout[0], dout[1], OUT))

A('            <ComObjectTable>')
for num, name, text, ft, dpt, size, flags in KOS:
    A('              <ComObject Id="%s" Number="%sK%d%s" Name="CH%s%s" Text="%s" FunctionText="GoodWe %s: %s" ObjectSize="%s" DatapointType="%s" %s />'
      % (oid(num), P, num, P, CH, name, xml(text), CH, xml(ft), size, dpt, flags))
A('            </ComObjectTable>')
A('            <ComObjectRefs>')
for num, name, text, ft, dpt, size, flags in KOS:
    A('              <ComObjectRef Id="%s" RefId="%s" Text="{{0:GoodWe %s}}: %s" FunctionText="GoodWe %s: %s" TextParameterRefId="%s" />'
      % (oref(num), oid(num), CH, xml(text), CH, xml(ft), NAMEREF))
A('            </ComObjectRefs>')
A('          </Static>')

# ---------------------------------------------------------------- Dynamic
A('          <Dynamic>')
A('            <ChannelIndependentBlock>')
A('              <!-- Eine eigene Tabelle je Kanal auf der Seite "Kanalauswahl". -->')
A('              <ParameterBlock Id="%sAID%s_PB-nnn" Name="Settings">' % (P, P))
A('                <ParameterBlock Id="%sAID%s_PB-nnn" Inline="true" Layout="Grid">' % (P, P))
A('                  <Rows>')
A('                    <Row Id="%sAID%s_PB-nnn_R-1" />' % (P, P))
A('                  </Rows>')
A('                  <Columns>')
A('                    <Column Id="%sAID%s_PB-nnn_C-1" Width="20%s" />' % (P, P, P))
A('                    <Column Id="%sAID%s_PB-nnn_C-2" Width="30%s" />' % (P, P, P))
A('                    <Column Id="%sAID%s_PB-nnn_C-3" Width="50%s" />' % (P, P, P))
A('                  </Columns>')
A('                  <ParameterSeparator Id="%s" Cell="1,1" Text="Kanal %s" />' % (PS, CH))
A('                  <!-- Haupttyp (mit Deaktiviert): nur hier wird ein Kanal aktiviert/deaktiviert -->')
A('                  <ParameterRefRef RefId="%s" Cell="1,2" HelpContext="GDW-Wechselrichtertyp" />' % TYPEREF)
A('                  <ParameterRefRef RefId="%s" Cell="1,3" HelpContext="BASE-ChannelName" />' % NAMEREF)
A('                </ParameterBlock>')
A('              </ParameterBlock>')
A('')
A('              <!-- Kanal-Tab, nur für aktivierte Kanäle (Haupttyp > 0). -->')
A('              <ParameterBlock Id="%sAID%s_PB-nnn" Name="Channel">' % (P, P))
A('                <choose ParamRefId="%s">' % TYPEREF)
A('                  <when test="&gt;0">')
A('                    <ParameterBlock Id="%sAID%s_PB-nnn" Name="Channel%sPage" Text="Kanal %s: {{0: ...}}" TextParameterRefId="%s" Icon="white-balance-sunny" ShowInComObjectTree="true" HelpContext="GDW-Wechselrichtertyp">' % (P, P, CH, CH, DISPLAYREF))
A('                      <ParameterSeparator Id="%s" Text="Kanaldefinition" UIHint="Headline" />' % PS)
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="BASE-ChannelName" />' % NAMEREF)
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Wechselrichtertyp" />' % TYPESELREF)
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="BASE-ChannelSuspended" />' % SUSPREF)
A('')
A('                      <ParameterSeparator Id="%s" Text="Verbindung" UIHint="Headline" />' % PS)
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Verbindung" />' % uref(1))
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Protokoll" />' % uref(3))
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Protokoll" />' % uref(2))
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Modbus-Adresse" />' % uref(4))
A('                      <ParameterSeparator Id="%s" Text="Nach Eingabe der IP-Adresse ermittelt &quot;Wechselrichter auslesen&quot; Protokoll, Typ und Modbus-Adresse und aktiviert alle Objekte, die das Gerät liefert. Das Gerät muss dazu programmiert und im Netz sein. Anschließend erneut programmieren." UIHint="Information" />' % PS)
A('                      <Button Id="%sAID%s_B-%sTT%s%sCC%s001" Text="Wechselrichter auslesen" EventHandler="GDW_readInverter" EventHandlerOnline="ConnectionOriented" EventHandlerParameters="{ &quot;channel&quot;:%s }" />'
  % (P, P, P, P, P, P, CH))
A('                      <ParameterSeparator Id="%s" Text="{{0:}}" TextParameterRefId="%s" />' % (PS, INFOREF))
A('')
A('                      <ParameterSeparator Id="%s" Text="Abfrage und Sendeverhalten" UIHint="Headline" />' % PS)
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Abfrage" />' % uref(5))
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Sendeverhalten" />' % uref(7))
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Sendeverhalten" />' % uref(6))
A('                      <ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Sendeverhalten" />' % uref(8))


def emit_ko_visibility(indent, k, ko_numbers, fam):
    pad = " " * indent
    if FAM_TEST[fam]:
        A(pad + '<choose ParamRefId="%s">' % TYPEREF)
        A(pad + '  <when test="%s">' % FAM_TEST[fam])
        pad2 = pad + "    "
    else:
        pad2 = pad
    A(pad2 + '<choose ParamRefId="%s">' % uref(bit_param(k)))
    A(pad2 + '  <when test="=1">')
    for num in ko_numbers:
        A(pad2 + '    <ComObjectRefRef RefId="%s" />' % oref(num))
    A(pad2 + '  </when>')
    A(pad2 + '</choose>')
    if FAM_TEST[fam]:
        A(pad + '  </when>')
        A(pad + '</choose>')


# KO-Sichtbarkeit: Haken gesetzt und Typ passend
for k, s in enumerate(SENSORS):
    emit_ko_visibility(22, k, [k], s[4])
for n, st in enumerate(SETTINGS):
    koIn, koOut = setting_ko(n)
    emit_ko_visibility(22, SENSOR_CAPACITY + n, [koIn] + ([koOut] if st[4] is not None else []), st[2])

A('')
A('                      <ParameterSeparator Id="%s" Text="Messwerte" UIHint="Headline" />' % PS)
A('                      <ParameterSeparator Id="%s" Text="Angehakt wird als KO angelegt und gesendet. &quot;Wechselrichter auslesen&quot; hakt alle Objekte an, die das Gerät liefert. Netzleistung, Bezug, Einspeisung und Hausverbrauch sind nur mit angeschlossenem GoodWe-Zähler gültig." UIHint="Information" />' % PS)


# Je Objekt eine normale Parameterzeile unter einer Gruppenueberschrift. Bewusst KEINE
# Tabelle: ein Layout="Table" mit nur einer Spalte zeigt in der ETS die Haekchen nicht an
# (die Zeilenkoepfe verdraengen die Spalte), und ohne weitere Spalten bringt es nichts.
def emit_list(title, bits, fam):
    """bits: Liste der Bitnummern in Anzeigereihenfolge."""
    pad = " " * 22
    if FAM_TEST[fam]:
        A(pad + '<choose ParamRefId="%s">' % TYPEREF)
        A(pad + '  <when test="%s">' % FAM_TEST[fam])
        pad = pad + "    "
    A(pad + '<ParameterSeparator Id="%s" Text="%s" UIHint="Headline" />' % (PS, xml(title)))
    for k in bits:
        A(pad + '<ParameterRefRef IndentLevel="1" RefId="%s" HelpContext="GDW-Objekte" />' % uref(bit_param(k)))
    if FAM_TEST[fam]:
        A(" " * 22 + '  </when>')
        A(" " * 22 + '</choose>')


k = 0
for title, group in SENSOR_GROUPS:
    bits = []
    fams = set()
    for s in group:
        bits.append(k)
        fams.add(s[4])
        k += 1
    assert len(fams) == 1, "Gruppe %s mischt Familien" % title
    emit_list(title, bits, fams.pop())
assert k == len(SENSORS)

A('')
A('                      <ParameterSeparator Id="%s" Text="Einstellungen" UIHint="Headline" />' % PS)
A('                      <ParameterSeparator Id="%s" Text="Diese Objekte schreiben in den Wechselrichter. Sie wirken erst, wenn ein Telegramm eintrifft; beim Start wird nichts geschrieben. Einstellungen ändern den Betrieb der Anlage - mit Bedacht verwenden." UIHint="Information" />' % PS)
for fam, title in (("C", "Einstellungen allgemein"), ("E", "Einstellungen Hybrid"), ("D", "Einstellungen netzgekoppelt")):
    bits = [SENSOR_CAPACITY + n for n, st in enumerate(SETTINGS) if st[2] == fam]
    if bits:
        emit_list(title, bits, fam)

A('                    </ParameterBlock>')
A('                  </when>')
A('                </choose>')
A('              </ParameterBlock>')
A('            </ChannelIndependentBlock>')
A('          </Dynamic>')
A('        </ApplicationProgram>')
A('      </ApplicationPrograms>')
A('    </Manufacturer>')
A('  </ManufacturerData>')
A('</KNX>')

io.open(os.path.join(SRC, "GoodWe.templ.xml"), "w", encoding="utf-8", newline="\n").write("\n".join(L) + "\n")

# ---------------------------------------------------------------- C++-Katalog


def encoding_of(dpt):
    d = dpt[0].replace("DPST-", "").split("-")
    main, sub = int(d[0]), int(d[1])
    if main == 14:
        enc = "EncFloat"
    elif main == 13:
        enc = "EncEnergyWh"
    elif main == 9:
        enc = "EncFloat2"
    elif main == 5 and sub == 1:
        enc = "EncPercent"
    elif main == 5:
        enc = "EncU8"
    elif main == 7:
        enc = "EncU16"
    elif main == 12:
        enc = "EncU32"
    elif main == 19:
        enc = "EncDateTime"
    elif main == 1:
        enc = "EncBool"
    else:
        raise ValueError(dpt)
    return enc, main, sub


def src_init(src):
    if src is None:
        return "{0, 0, TypeNone, 0}"
    reg, typ, div, reg2 = src
    if typ == "CALC":
        return "{0, 0, TypeCalc, Calc%s}" % "".join(w.capitalize() for w in CALCS[div].split("_"))
    if typ == "CALCUI":
        return "{%d, %d, TypeCalcUI, 1}" % (reg, reg2)
    return "{%d, %d, Type%s, %d}" % (reg, reg2, typ, DIVS.index(div))


H = []
B = H.append
B('#pragma once')
B('#include <stdint.h>')
B('')
B('// ERZEUGT von tools/gen_templ.py aus tools/slots.py - nicht von Hand bearbeiten.')
B('//')
B('// Objektkatalog: Herkunft (Register, Datentyp, Teiler) je Geraetefamilie und KO-Kodierung.')
B('// Die Reihenfolge ist die KO-Reihenfolge und die Lage der Aktiv-Bits.')
B('')
B('namespace GoodWe')
B('{')
B('    enum ValueType : uint8_t')
B('    {')
B('        TypeNone = 0,')
for t in TYPES:
    B('        Type%s,' % {"CALC": "Calc", "CALCUI": "CalcUI"}.get(t, t))
B('    };')
B('')
B('    enum CalcId : uint8_t')
B('    {')
for c in CALCS:
    B('        Calc%s,' % "".join(w.capitalize() for w in c.split("_")))
B('    };')
B('')
B('    enum Encoding : uint8_t')
B('    {')
B('        EncFloat,     // DPT 14.x')
B('        EncFloat2,    // DPT 9.x')
B('        EncEnergyWh,  // DPT 13.010, Wert in kWh -> Wh ganzzahlig')
B('        EncPercent,   // DPT 5.001')
B('        EncU8,        // DPT 5.010')
B('        EncU16,       // DPT 7.001')
B('        EncU32,       // DPT 12.001')
B('        EncDateTime,  // DPT 19.001')
B('        EncBool,      // DPT 1.x')
B('    };')
B('')
B('    // Bedingungen fuer die Verfuegbarkeit, Bitmaske (siehe tools/slots.py)')
B('    enum Cond : uint16_t')
B('    {')
for i, c in enumerate(COND):
    B('        Cond%s = 1 << %d,' % (c, i))
B('    };')
B('')
B('    // Familienmaske')
B('    static const uint8_t FAM_ET = 1;')
B('    static const uint8_t FAM_DT = 2;')
B('')
B('    // Teiler zum Feld "div" einer Quelle')
B('    extern const float DIV[%d];' % len(DIVS))
B('')
B('    struct Source')
B('    {')
B('        uint16_t reg;   // Register (bei TypeCalcUI: Spannung)')
B('        uint16_t reg2;  // zweites Register (TypeBITS22: Low-Word, TypeCalcUI: Strom)')
B('        uint8_t type;   // ValueType')
B('        uint8_t div;    // Index in DIV, bei TypeCalc die CalcId')
B('    };')
B('')
B('    struct Sensor')
B('    {')
B('        const char* name;')
B('        Source et;')
B('        Source dt;')
B('        uint16_t cond;')
B('        uint8_t encoding;')
B('        uint8_t dptMain;')
B('        uint8_t dptSub;')
B('    };')
B('')
B('    struct Setting')
B('    {')
B('        const char* name;')
B('        uint8_t families;   // FAM_ET | FAM_DT')
B('        uint8_t encIn;')
B('        uint8_t dptInMain;')
B('        uint8_t dptInSub;')
B('        uint8_t encOut;     // nur gueltig, wenn hasStatus')
B('        uint8_t dptOutMain;')
B('        uint8_t dptOutSub;')
B('        bool hasStatus;')
B('    };')
B('')
B('    // Parameterlayout je Kanal, muss zu tools/gen_templ.py passen')
B('    static const uint16_t ENABLE_OFFSET = %d;' % ENABLE_OFFSET)
B('    static const uint16_t SENSOR_COUNT = %d;' % len(SENSORS))
B('    static const uint16_t SENSOR_CAPACITY = %d;' % SENSOR_CAPACITY)
B('    static const uint16_t SETTING_COUNT = %d;' % len(SETTINGS))
B('    static const uint16_t SETTING_KO_BASE = %d;' % SETTING_KO_BASE)
B('    static const uint16_t ENABLE_BITS = %d;' % ENABLE_BITS)
B('')
B('    enum SettingId : uint8_t')
B('    {')
for n, st in enumerate(SETTINGS):
    B('        Set%s = %d,' % (st[0], n))
B('    };')
B('')
B('    extern const Sensor SENSORS[SENSOR_COUNT];')
B('    extern const Setting SETTINGS[SETTING_COUNT];')
B('} // namespace GoodWe')

io.open(os.path.join(SRC, "SlotCatalog.h"), "w", encoding="utf-8", newline="\n").write("\n".join(H) + "\n")

# Die Tabellen selbst in einer eigenen Uebersetzungseinheit, damit sie nur einmal im Flash liegen.
H = []
B = H.append
B('// ERZEUGT von tools/gen_templ.py aus tools/slots.py - nicht von Hand bearbeiten.')
B('#include "SlotCatalog.h"')
B('')
B('namespace GoodWe')
B('{')
B('    const float DIV[%d] = {%s};' % (len(DIVS), ", ".join("%d.0f" % d for d in DIVS)))
B('')
B('    const Sensor SENSORS[SENSOR_COUNT] = {')
for k, s in enumerate(SENSORS):
    name, label, unit, dpt, fam, et, dt, cond = s
    enc, main, sub = encoding_of(dpt)
    B('        {"%s", %s, %s, 0x%03X, %s, %d, %d}, // %d' % (name, src_init(et), src_init(dt), cond_mask(cond), enc, main, sub, k))
B('    };')
B('')
B('    const Setting SETTINGS[SETTING_COUNT] = {')
for n, st in enumerate(SETTINGS):
    name, label, fam, din, dout, desc = st
    ei, mi, si = encoding_of(din)
    if dout is not None:
        eo, mo, so = encoding_of(dout)
        has = "true"
    else:
        eo, mo, so = "EncBool", 0, 0
        has = "false"
    B('        {"%s", %d, %s, %d, %d, %s, %d, %d, %s},' % (name, FAM_MASK[fam], ei, mi, si, eo, mo, so, has))
B('    };')
B('} // namespace GoodWe')
io.open(os.path.join(SRC, "SlotCatalog.cpp"), "w", encoding="utf-8", newline="\n").write("\n".join(H) + "\n")

# ---------------------------------------------------------------- ETS-Skript
names = [""] * ENABLE_BITS
for k, name, _, _ in BITS:
    names[k] = name
lines = []
row = []
for i, nm in enumerate(names):
    row.append('"%s"' % nm)
    if len(row) == 8:
        lines.append("    " + ", ".join(row))
        row = []
if row:
    lines.append("    " + ", ".join(row))
template = io.open(os.path.join(HERE, "script.template.js"), encoding="utf-8").read()
script = template.replace("/*BITNAMES*/", ",\n".join(lines))
io.open(os.path.join(SRC, "GoodWe.script.js"), "w", encoding="utf-8", newline="\n").write(
    "// ERZEUGT von tools/gen_templ.py - nicht von Hand bearbeiten. Logik: tools/script.template.js\n" + script)

print("templ.xml: %d Messwerte, %d Einstellungen, %d KOs je Kanal (hoechste Nummer %d), Block %d Byte"
      % (len(SENSORS), len(SETTINGS), len(KOS), max(k[0] for k in KOS), BLOCK_BYTES))
