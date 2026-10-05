# Agregar un navegador

*[Read in English](../en/ADDING_A_BROWSER.md)*

trc-wap-relay parte con Neomar, pero el motor de reenvío no sabe nada de
ningún navegador en particular. Soportar otro consiste principalmente en
averiguar a dónde llama y dejarlo escrito como un perfil.

## ¿El navegador necesita este relay?

Solo lo necesitan los navegadores cuyo gateway **no se puede cambiar**. Si
el navegador permite escribir la dirección del gateway (muchos navegadores
WAP 1.x lo permiten, en la configuración de *gateway WAP* / *proxy*),
apúntalo directamente a un gateway que hable su protocolo y no hace falta
ningún relay. (El servidor Boat Rudder de The Retro Center puede responder
WSP estándar sin conexión en UDP 9200, pero ese puerto está desactivado por
ahora en theretrocenter.com - es experimental y no se ha probado con
teléfonos reales.)

## 1. Averigua a dónde llama

Con el dispositivo conectado a través del PC y el enlace PPP activo:

- **Softick PPP:** *Tools → Log*, luego carga una página en el dispositivo y
  lee el destino de las líneas `UDP_IN`.
- **Cualquier otro:** captura la tarjeta de red del PC con Wireshark
  (filtro `udp or tcp`) mientras cargas una página.

Anota cada dirección de destino, el puerto, y si es UDP o TCP. Prueba un
par de veces - algunos navegadores alternan entre un gateway primario y uno
secundario.

Si el navegador usa **TCP**, ver [Navegadores TCP](#navegadores-tcp) más abajo.

## 2. Asegúrate de que un gateway hable su protocolo

El relay reenvía bytes; el gateway remoto tiene que entenderlos. Los
navegadores WAP 1.x estándar envían WSP sin conexión (UDP 9200) o WTP+WSP
(UDP 9201); otros, como Neomar, tienen sus propias particularidades.
Verifica que el gateway al que vas a apuntar (el servidor Boat Rudder de
The Retro Center, o el tuyo) maneje ese protocolo, y en qué puerto.

Antes de escribir código ya puedes probar con el perfil existente y las
opciones de reemplazo:

```
trc-wap-relay.exe -b neomar -l <ip>,<ip> -P <puerto-al-que-llama> -p <puerto-del-gateway>
```

(`-l` y `-P` reemplazan las direcciones y el puerto donde responder, `-p`
el puerto del gateway.)

## 3. Agrega el perfil

En [`src/browsers.c`](../../src/browsers.c), agrega una entrada y súmala a
`BROWSERS[]`:

```c
static const BrowserProfile EXAMPLE = {
    .id           = "example",                    /* se usa con -b, en minúsculas */
    .name         = "Example Browser 2.0",
    .device       = "Psion Series 5mx (EPOC R5)",
    .addresses    = { "192.0.2.10", "192.0.2.11", NULL },
    .port         = 9201,                         /* a lo que llama el navegador */
    .gateway_port = 9201,                         /* donde responde el gateway */
    .protocol     = "WTP + WSP over UDP",
};

const BrowserProfile *const BROWSERS[] = {
    &NEOMAR,
    &EXAMPLE,
    NULL
};
```

Un comentario sobre la entrada que diga de dónde salieron las direcciones
(qué dispositivo, qué versión, cómo se capturaron) ayuda a la próxima
persona cuya versión sea distinta.

## 4. Documéntalo y pruébalo

- Agrega una guía en `docs/en/browsers/<id>.md` y
  `docs/es/navegadores/<id>.md` (usa [neomar.md](navegadores/neomar.md) como
  plantilla), y una fila en la tabla *Navegadores soportados* de ambos README.
- `make test` debe seguir pasando. Si el navegador tiene su propio framing,
  se agradece una variante de `tests/fake_device.py` / `tests/fake_gateway.py`
  que lo hable.
- Anota en `CHANGELOG.md` qué se probó y en qué hardware.

Se pueden ejecutar varios perfiles a la vez: `trc-wap-relay.exe -b neomar,example`.

## Navegadores TCP

Los navegadores que llegan a su gateway por TCP (por ejemplo, navegadores
WAP 2.0 que usan un proxy HTTP en una dirección fija) necesitan un reenvío
TCP, que todavía no existe. La forma prevista:

- un campo `transport` en `BrowserProfile` (`TRANSPORT_UDP` /
  `TRANSPORT_TCP`);
- un `relay_tcp.c` junto a `relay.c`, que acepte conexiones en la dirección
  agregada y las conecte con el gateway - `netaddr.c` sirve sin cambios;
- `main.c` creando las escuchas según el transporte, y ambos reenvíos
  atendidos desde el mismo bucle `select()` (ojo con el `FD_SETSIZE` de 64
  sockets).
