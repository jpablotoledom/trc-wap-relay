# Neomar / Rover 1.5 (Palm OS)

*[Read in English](../../en/browsers/neomar.md)*

| | |
|---|---|
| Perfil | `neomar` (por defecto) |
| Navegador | Neomar, internamente *Rover 1.5*, incluido en algunos CD de software oficiales de Palm OS |
| Probado en | Palm m105, Palm OS 3.5.3 |
| Llama a | `165.160.13.20` y `165.160.15.20`, UDP `49300` |
| Protocolo | WTP sobre UDP (el framing propio de Rover) con WSP GET; las respuestas son WBXML |
| Estado | ✅ Funciona de punta a punta: Palm m105 → serial → Windows XP SP3 + MochaPPP → internet → theretrocenter.com |

## Qué necesitas

- Un dispositivo Palm OS con Neomar instalado.
- Un PC con Windows (XP o posterior) al que se conecte la Palm - cuna serial o USB.
- Software servidor PPP en el PC: **MochaPPP** (probado) o **Softick PPP**.
- Acceso a internet en el PC.
- `trc-wap-relay.exe`.

## 1. Deja funcionando el enlace PPP

Configura MochaPPP o Softick PPP como lo harías normalmente para que la Palm
tenga conexión TCP/IP a través del PC (Prefs → Network en la Palm, con la
conexión que indique tu software PPP). Neomar se conecta por dirección IP,
así que la configuración de DNS de la Palm no importa para esto.

## 2. Ejecuta el relay

En el PC, como Administrador:

```
trc-wap-relay.exe
```

Resuelve el gateway, agrega `165.160.13.20` y `165.160.15.20` a la conexión
a internet del PC y empieza a escuchar:

```
[14:31:10] Gateway: theretrocenter.com (167.114.153.137)
[14:31:10] Added 165.160.13.20/255.255.255.248 to Intel(R) PRO/Wireless 3945ABG Network Connection (DHCP)
[14:31:10] [neomar] Listening on 165.160.13.20:49300 -> gateway 167.114.153.137:49300
[14:31:10] Added 165.160.15.20/255.255.255.248 to Intel(R) PRO/Wireless 3945ABG Network Connection (DHCP)
[14:31:11] [neomar] Listening on 165.160.15.20:49300 -> gateway 167.114.153.137:49300
[14:31:11] Ready. Open the browser on the device and load a page. Ctrl+C to quit.
```

Permítelo si el Firewall de Windows pregunta.

## 3. Navega

Abre Neomar en la Palm y presiona **Start**. En la ventana del relay:

```
[14:31:42] [neomar] New session: device 165.160.13.20:1026 -> 165.160.13.20:49300 -> gateway 167.114.153.137:49300
[14:31:42] [neomar] device 165.160.13.20:1026 -> gateway: 92 bytes
[14:31:43] [neomar] gateway -> device 165.160.13.20:1026: 812 bytes
```

(El dispositivo aparece con una dirección local porque el software PPP
envía en su nombre desde el PC.)

## Confirmar las direcciones de tu versión de Rover

Las dos direcciones se observaron en una versión de Rover 1.5; otras pueden
ser distintas. Si el relay nunca registra `New session` al presionar Start:

- **Softick PPP:** activa *Tools → Log*, presiona Start en la Palm y busca
  una línea como `UDP_IN: 10.0.0.1:xxxx -> <ip>:<puerto>`.
- **MochaPPP:** captura la tarjeta de red del PC con Wireshark (filtro
  `udp`) mientras presionas Start, y lee la dirección y el puerto de destino.

Luego ejecuta el relay con lo que encontraste:

```
trc-wap-relay.exe -b neomar -l <ip>[,<ip>...] [-P <puerto>]
```

y considera abrir un issue para actualizar el perfil.

## Si venías del método de ruta/ARP

Si seguiste la configuración original de neomar-wap-proxy (ruta estática +
entrada ARP estática hacia un gateway en tu LAN), borra esas entradas
primero:

```bat
route delete 165.160.13.20
route delete 165.160.15.20
arp -d 165.160.13.20
arp -d 165.160.15.20
```

## Del lado del gateway

Rover solo acepta un WTP Result que refleje los primeros cuatro bytes de su
Invoke (TID, el byte final, incluso el flag RID) con el tipo de PDU cambiado
a Result, y no reensambla de forma confiable respuestas segmentadas, así que
las páginas deben caber en un datagrama. El gateway de The Retro Center
maneja ambas cosas; para montar el tuyo, ver
[neomar-wap-proxy](https://github.com/jpablotoledom/neomar-wap-proxy) y su
`docs/PROTOCOLO.md`, y apunta el relay a él con `-t`.
