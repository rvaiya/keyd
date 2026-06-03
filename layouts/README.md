# Keyboard Layouts

keyd ships with a set of common keyboard layouts in `/usr/share/keyd/layouts/`.
These can be included in your config to remap alpha keys.

## Usage

```ini
[main]
include layouts/dvorak
```

Or set as the default in a `[global]` section:

```ini
[global]
default_layout = dvorak
```

## Layout Types

### Alternative QWERTY Layouts

| Layout | Description |
|---|---|
| [colemak](colemak) | Colemak layout |
| [dvorak](dvorak) | Dvorak Simplified Keyboard |
| [workman](workman) | Workman (stakev) layout |
| [graphite](graphite) | Graphite layout |
| [graphite-angle-kp](graphite-angle-kp) | Graphite variant with angle + KP |

### National Layouts (by ISO country code)

| Code | Country | Code | Country | Code | Country |
|---|---|---|---|---|---|
| af | Afghanistan | dk | Denmark | ie | Ireland |
| al | Albania | dz | Algeria | il | Israel |
| am | Armenia | ee | Estonia | in | India |
| ara | Arabic | es | Spain | iq | Iraq |
| at | Austria | et | Ethiopia | ir | Iran |
| au | Australia | fi | Finland | is | Iceland |
| az | Azerbaijan | fo | Faroe Islands | it | Italy |
| ba | Bosnia & Herzegovina | fr | France | jp | Japan |
| bd | Bangladesh | gb | United Kingdom | jv | Java |
| be | Belgium | ge | Georgia | ke | Kenya |
| bg | Bulgaria | gh | Ghana | kg | Kyrgyzstan |
| br | Brazil | gn | Guinea | kh | Cambodia |
| bt | Bhutan | gr | Greece | kr | South Korea |
| bw | Botswana | hr | Croatia | kz | Kazakhstan |
| by | Belarus | hu | Hungary | la | Laos |
| ca | Canada | id | Indonesia | latam | Latin America |
| cd | Dem. Rep. Congo | ie | Ireland | lk | Sri Lanka |
| ch | Switzerland | il | Israel | lt | Lithuania |
| cm | Cameroon | in | India | lv | Latvia |
| cn | China | iq | Iraq | ma | Morocco |
| cz | Czech Republic | ir | Iran | mao | Maori |
| de | Germany | is | Iceland | md | Moldova |
| dk | Denmark | it | Italy | me | Montenegro |
| dz | Algeria | jp | Japan | mk | North Macedonia |
| ee | Estonia | jv | Java | ml | Mali |
| es | Spain | ke | Kenya | mm | Myanmar |
| et | Ethiopia | kg | Kyrgyzstan | mn | Mongolia |
| fi | Finland | kh | Cambodia | mt | Malta |
| fo | Faroe Islands | kr | South Korea | mv | Maldives |
| fr | France | kz | Kazakhstan | my | Malaysia |
| gb | United Kingdom | la | Laos | ng | Nigeria |
| ge | Georgia | lk | Sri Lanka | nl | Netherlands |
| gh | Ghana | lt | Lithuania | no | Norway |
| gn | Guinea | lv | Latvia | np | Nepal |
| gr | Greece | ma | Morocco | ph | Philippines |
| hr | Croatia | mao | Maori | pk | Pakistan |
| hu | Hungary | md | Moldova | pl | Poland |
| id | Indonesia | me | Montenegro | pt | Portugal |
| ie | Ireland | mk | North Macedonia | ro | Romania |
| il | Israel | ml | Mali | rs | Serbia |
| in | India | mm | Myanmar | ru | Russia |
| iq | Iraq | mn | Mongolia | se | Sweden |
| ir | Iran | mt | Malta | si | Slovenia |
| is | Iceland | mv | Maldives | sk | Slovakia |
| it | Italy | my | Malaysia | sn | Senegal |
| jp | Japan | ng | Nigeria | sy | Syria |
| jv | Java | nl | Netherlands | tg | Togo |
| ke | Kenya | no | Norway | th | Thailand |
| kg | Kyrgyzstan | np | Nepal | tj | Tajikistan |
| kh | Cambodia | ph | Philippines | tm | Turkmenistan |
| kr | South Korea | pk | Pakistan | tr | Turkey |
| kz | Kazakhstan | pl | Poland | tw | Taiwan |
| la | Laos | pt | Portugal | tz | Tanzania |
| lk | Sri Lanka | ro | Romania | ua | Ukraine |
| lt | Lithuania | rs | Serbia | uz | Uzbekistan |
| lv | Latvia | ru | Russia | vn | Vietnam |
| ma | Morocco | se | Sweden | za | South Africa |
| mao | Maori | si | Slovenia | | |
| md | Moldova | sk | Slovakia | | |
| me | Montenegro | sn | Senegal | | |
| mk | North Macedonia | sy | Syria | | |
| ml | Mali | tg | Togo | | |
| mm | Myanmar | th | Thailand | | |
| mn | Mongolia | tj | Tajikistan | | |
| mt | Malta | tm | Turkmenistan | | |
| mv | Maldives | tr | Turkey | | |
| my | Malaysia | tw | Taiwan | | |
| ng | Nigeria | tz | Tanzania | | |
| nl | Netherlands | ua | Ukraine | | |
| no | Norway | uz | Uzbekistan | | |
| np | Nepal | vn | Vietnam | | |
| ph | Philippines | za | South Africa | | |

### Accessibility

| Layout | Description |
|---|---|
| [brai](brai) | Braille layout |

### Special Purpose

| Layout | Description |
|---|---|
| [epo](epo) | Esperanto (interlingua QWERTY) |

## Important Notes

- Non-English layouts include a dedicated shift layer and **require keyd's compose definitions** (see [docs/troubleshooting.md](../docs/troubleshooting.md)).
- Inspect layout files before including them — understand the mappings.
- Shipped layouts define `[layoutname:layout]` sections exclusively.
- For country layouts, the layout name matches the ISO country code (e.g. `[de:layout]`).
