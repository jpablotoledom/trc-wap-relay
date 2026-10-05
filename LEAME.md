# TRC WAP Relay

*[Read in English](README.md)*

Devuelve a internet los navegadores WAP antiguos desde el PC con Windows al
que se conectan - sin hardware extra ni trucos en el router.

Los navegadores WAP antiguos no buscan su gateway: la dirección IP del
operador viene grabada en el navegador, y esos gateways se apagaron hace
veinte años. **trc-wap-relay** es un único `.exe` pequeño (≈50 KB, sin
dependencias, corre en **Windows XP** y posteriores) que hace que el PC
responda en esas direcciones muertas y reenvía el tráfico a un gateway WAP
moderno en internet - el de [The Retro Center](https://theretrocenter.com)
por defecto.

```
Palm m105 ── serial ── Windows XP ──────────── internet ──────────── theretrocenter.com
 Neomar                 MochaPPP                                      gateway WAP
                        trc-wap-relay  (responde en 165.160.13.20)    UDP 49300
```

## Navegadores soportados

| Perfil | Navegador | Dispositivo | Estado |
|---|---|---|---|
| `neomar` | Neomar / Rover 1.5 | Palm OS (Palm m105, Palm OS 3.5.3) | ✅ Probado de punta a punta en hardware real |

El proyecto está pensado para crecer: cada navegador es un *perfil*
pequeño (las direcciones y el puerto a los que llama), y el motor de
reenvío es compartido. Ver [Agregar un navegador](docs/es/AGREGAR_UN_NAVEGADOR.md).

## Inicio rápido (Neomar en una Palm)

1. Conecta la Palm al PC con Windows y consigue un enlace PPP funcionando
   (MochaPPP, Softick PPP, …) - ver [la guía de Neomar](docs/es/navegadores/neomar.md).
2. Copia `trc-wap-relay.exe` al PC y ejecútalo **como Administrador**:
   ```
   trc-wap-relay.exe
   ```
   Deberías ver:
   ```
   [14:31:10] Gateway: theretrocenter.com (167.114.153.137)
   [14:31:10] Added 165.160.13.20/255.255.255.248 to Intel(R) PRO/Wireless 3945ABG ... (DHCP)
   [14:31:10] [neomar] Listening on 165.160.13.20:49300 -> gateway 167.114.153.137:49300
   ...
   [14:31:11] Ready. Open the browser on the device and load a page. Ctrl+C to quit.
   ```
3. Abre Neomar en la Palm y presiona **Start**.
4. Sal con **Ctrl+C**: se quitan las direcciones que agregó el relay.

## Uso

```
trc-wap-relay.exe [-b navegador[,navegador...]] [-t host] [-p puerto] [-l ip,ip,...] [-P port] [-n] [-q]
trc-wap-relay.exe -L | -h | -V

  -b  perfil(es) de navegador a atender   (por defecto neomar; -L los lista)
  -t  host del gateway WAP remoto         (por defecto theretrocenter.com)
  -p  puerto del gateway, en lugar del del perfil (solo un perfil)
  -l  direcciones donde responder, en lugar de las del perfil (solo un perfil)
  -P  puerto donde responder, en lugar del del perfil (solo un perfil)
  -n  no agregar direcciones faltantes a este PC; deben existir ya
  -q  silencioso: no registra cada datagrama
  -L  lista los navegadores soportados
  -V  muestra la versión
```

Ejemplos:

```
trc-wap-relay.exe -t mi-gateway.example.com      usar tu propio gateway
trc-wap-relay.exe -q                             solo registra sesiones y errores
trc-wap-relay.exe -n                             direcciones configuradas a mano (sin permisos de admin)
```

El gateway remoto debe hablar el protocolo del navegador y ser accesible
en el puerto UDP del perfil. The Retro Center tiene uno; para montar el
tuyo, ver [neomar-wap-proxy](https://github.com/jpablotoledom/neomar-wap-proxy)
(Python) o el servidor Boat Rudder de The Retro Center.

## Cómo funciona

Windows XP no puede reescribir el destino de un paquete, así que en su
lugar el PC **se adueña** de las direcciones muertas del gateway del
navegador mientras el relay corre. El software PPP le entrega el tráfico
del dispositivo a Windows, Windows entrega todo lo dirigido a una dirección
local a quien escuche ahí - el relay - y el relay lo reenvía al gateway
real, devolviendo las respuestas **desde la dirección a la que llamó el
dispositivo** (los clientes WAP ignoran cualquier otra).

Detalles, diagramas y decisiones de diseño: [Arquitectura](docs/es/ARQUITECTURA.md).

## Compilación

Desde Linux, con el compilador cruzado MinGW-w64:

```bash
sudo apt install gcc-mingw-w64-i686 make
./build.sh                 # -> dist/trc-wap-relay.exe
```

En Windows con MinGW: `make CC=gcc`.

La compilación fija la cabecera PE en la versión 5.01 para que Windows XP
la acepte, enlaza estáticamente, y el código solo usa APIs presentes en XP
(Winsock 2, IP Helper).

## Pruebas

```bash
sudo apt install wine python3
make test
```

Ejecuta el `.exe` bajo Wine contra un gateway y un dispositivo falsos en
direcciones de loopback: línea de comandos, respuestas desde la dirección
llamada, cabeceras reflejadas, datagramas de más de 1200 bytes y una sesión
por cada par del dispositivo. Dos caminos necesitan un Windows real y están
verificados ahí: agregar direcciones (Wine no implementa `AddIPAddress`) e
informar un gateway que rechaza el datagrama (Wine no reporta errores ICMP).

## Estructura del proyecto

```
trc-wap-relay/
├── src/
│   ├── main.c        línea de comandos, arranque y cierre
│   ├── browsers.c/h  perfiles de navegador (direcciones y puertos a los que llama cada uno)
│   ├── relay.c/h     motor de reenvío UDP: escuchas, sesiones, reenvío
│   ├── netaddr.c/h   agregar/quitar direcciones en Windows (API IP Helper)
│   └── log.c/h       log de consola con hora
├── tests/            pruebas con Wine, con gateway y dispositivo falsos
├── docs/
│   ├── en/           arquitectura, agregar un navegador, problemas, guías por navegador
│   ├── es/           lo mismo, en español
│   └── diagrams/     fuentes PlantUML
├── Makefile, build.sh
└── README.md / LEAME.md, CHANGELOG.md, LICENSE
```

## Documentación

- [Arquitectura](docs/es/ARQUITECTURA.md)
- [Guía de Neomar / Rover 1.5](docs/es/navegadores/neomar.md)
- [Solución de problemas](docs/es/SOLUCION_DE_PROBLEMAS.md)
- [Agregar un navegador](docs/es/AGREGAR_UN_NAVEGADOR.md)

## Créditos

El protocolo de Neomar/Rover se obtuvo por ingeniería inversa en
[neomar-wap-proxy](https://github.com/jpablotoledom/neomar-wap-proxy), donde
este relay nació como `tools/xp-relay`.

## Contacto

**Jonathan Pablo Toledo Moya**
theretrocenter.com@gmail.com

## Licencia

MIT - ver [LICENSE](LICENSE).
