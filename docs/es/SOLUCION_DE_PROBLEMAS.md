# Solución de problemas

*[Read in English](../en/TROUBLESHOOTING.md)*

El log del relay normalmente dice dónde se corta la cadena. Léelo de arriba
hacia abajo. Los mensajes del programa están en inglés; aquí van tal cual.

## Al arrancar

| Línea del log | Significado | Qué hacer |
|---|---|---|
| `cannot resolve theretrocenter.com` | El PC no tiene internet/DNS funcionando. | Revisa la conexión del PC; o usa `-t <dirección-ip>`. |
| `cannot add 165.160.13.20 - run trc-wap-relay as Administrator` | Se negó el permiso para agregar direcciones. | Clic derecho → *Ejecutar como…* con una cuenta de administrador, o configura las direcciones a mano y usa `-n` (más abajo). |
| `cannot add ... (error N)` | Windows rechazó la dirección en esa conexión. La línea indica el adaptador y si usa DHCP. | Prueba `-n` con las direcciones en el Adaptador de bucle invertido de Microsoft (más abajo), e informa el número de error. |
| `is not an address of this PC - configure it by hand, or drop -n` | Con `-n`, la dirección no está en ningún adaptador. | Configúrala (más abajo), o ejecuta sin `-n`. |
| `... is already in use by another program` | Otro programa escucha en esa dirección y puerto - a menudo una segunda copia del relay. | Cierra el otro programa. |
| `Nothing to listen on - exiting.` | Fallaron todas las direcciones; los motivos están en las líneas anteriores. | |

## Al cargar una página

| Lo que ves | Significado | Qué hacer |
|---|---|---|
| Nunca aparece `New session` | El tráfico del dispositivo no llega al relay. | Verifica que el enlace PPP funcione; confirma que el navegador realmente llame a las direcciones y el puerto del perfil (ver su guía - p. ej. [Neomar](navegadores/neomar.md#confirmar-las-direcciones-de-tu-versión-de-rover)). |
| `device -> gateway`, pero nunca `gateway -> device` | El gateway no responde. | Firewall en el servidor del gateway, el servicio del gateway detenido, o el puerto equivocado (`-p`). |
| `the gateway refused the datagram - is UDP N open on the server?` | Volvió un ICMP "port unreachable": se llega al servidor, pero nada escucha en ese puerto UDP. | Inicia/activa el gateway en el servidor, o corrige el puerto. |
| Líneas `gateway -> device` pero el dispositivo muestra un error | El gateway respondió algo que el navegador no acepta. | Problema del lado del gateway: revisa sus logs y las notas de protocolo del navegador. |

## Configurar las direcciones a mano (`-n`)

Útil sin permisos de administrador, o si Windows se niega a agregar las
direcciones automáticamente.

1. Agrega el **Adaptador de bucle invertido de Microsoft**: Panel de
   control → Agregar hardware → "Sí, ya conecté el hardware" → "Agregar un
   nuevo dispositivo de hardware" → "Instalar el hardware seleccionado
   manualmente de una lista" → Adaptadores de red → Microsoft →
   **Adaptador de bucle invertido de Microsoft**.
2. Busca su nombre de conexión con `ipconfig /all`, y luego:
   ```bat
   netsh interface ip set address name="Conexión de área local 2" static 165.160.13.20 255.255.255.0
   netsh interface ip add address name="Conexión de área local 2" 165.160.15.20 255.255.255.0
   ```
3. Ejecuta `trc-wap-relay.exe -n`.

Estas direcciones quedan hasta que las quites del adaptador.

## Direcciones que quedaron

Si el relay se cerró a la fuerza en lugar de con Ctrl+C, las direcciones
que agregó quedan hasta el próximo reinicio o hasta que esa conexión se
reinicie. No hacen daño, y la siguiente ejecución las reutiliza. Para
quitarlas de inmediato, deshabilita y vuelve a habilitar la conexión.
