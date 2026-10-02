<<<<<<< Updated upstream
# N.E.R.A.

### Neuro Estimulación de Relajación Asistida

**Sistema de asistencia para la relajación mediante campos electromagnéticos pulsados (PEMF).**

<p align="center">
  <img src="docs/nera.png" width="500" alt="N.E.R.A. - Sistema de asistencia para la relajación">
</p>
=======
N.E.R.A.
Neuro Estimulación de Relajación Asistida
Sistema de asistencia para la relajación mediante campos electromagnéticos pulsados (PEMF).

N.E.R.A. - Sistema de asistencia para la relajación

¿Qué es N.E.R.A.?
N.E.R.A. es un dispositivo portátil en desarrollo, diseñado para explorar el uso de campos electromagnéticos pulsados (PEMF) como herramienta de apoyo para la relajación y el manejo del estrés.

El proyecto está orientado principalmente a personas que experimentan situaciones de estrés, sobrecarga mental o tensión. También se contemplan posibles aplicaciones relacionadas con el descanso y la recuperación física.

La propuesta consiste en integrar una vincha con bobinas electromagnéticas, un sistema electrónico de control y un reloj encargado del monitoreo. Estos componentes conforman un sistema que busca utilizar la información obtenida durante el monitoreo para gestionar el funcionamiento de la estimulación.
>>>>>>> Stashed changes

El desarrollo combina electrónica, programación y diseño mecánico con el objetivo de crear un dispositivo portátil, compacto y no invasivo.

<<<<<<< Updated upstream
## ¿Qué es N.E.R.A.?

N.E.R.A. es un dispositivo portátil en desarrollo, diseñado para explorar el uso de campos electromagnéticos pulsados (PEMF) como herramienta de apoyo para la relajación y el manejo del estrés.

El proyecto está orientado principalmente a personas que experimentan situaciones de estrés, sobrecarga mental o tensión. También se contemplan posibles aplicaciones relacionadas con el descanso y la recuperación física.

La propuesta consiste en integrar una vincha con bobinas electromagnéticas, un sistema electrónico de control y un reloj encargado del monitoreo. Estos componentes conforman un sistema que busca utilizar la información obtenida durante el monitoreo para gestionar el funcionamiento de la estimulación.

El desarrollo combina electrónica, programación y diseño mecánico con el objetivo de crear un dispositivo portátil, compacto y no invasivo.
=======
¿Cómo funciona?
El funcionamiento de N.E.R.A. se basa en la generación de campos electromagnéticos pulsados mediante bobinas.

El reloj realiza el monitoreo y se comunica con el microcontrolador ubicado en la vincha. El microcontrolador procesa la información recibida y gestiona el funcionamiento del sistema PEMF.
>>>>>>> Stashed changes

Cuando circula corriente eléctrica por las bobinas, se genera un campo magnético. Al modificar la corriente, el campo magnético también varía, produciendo pulsos electromagnéticos.

<<<<<<< Updated upstream
## ¿Cómo funciona?

El funcionamiento de N.E.R.A. se basa en la generación de campos electromagnéticos pulsados mediante bobinas.

El reloj realiza el monitoreo y se comunica con el microcontrolador ubicado en la vincha. El microcontrolador procesa la información recibida y gestiona el funcionamiento del sistema PEMF.

Cuando circula corriente eléctrica por las bobinas, se genera un campo magnético. Al modificar la corriente, el campo magnético también varía, produciendo pulsos electromagnéticos.

Las bobinas se encuentran ubicadas detrás de las orejas y forman parte del sistema de estimulación de la vincha.

El objetivo del proyecto es explorar la utilización de esta tecnología como herramienta de apoyo para favorecer la relajación y contribuir al bienestar del usuario.

---

## Arquitectura del sistema

N.E.R.A. está compuesto por diferentes módulos que trabajan de manera conjunta.

### 1. N.E.R.A. Headband — Vincha PEMF

=======
Las bobinas se encuentran ubicadas detrás de las orejas y forman parte del sistema de estimulación de la vincha.

El objetivo del proyecto es explorar la utilización de esta tecnología como herramienta de apoyo para favorecer la relajación y contribuir al bienestar del usuario.

Arquitectura del sistema
N.E.R.A. está compuesto por diferentes módulos que trabajan de manera conjunta.

1. N.E.R.A. Headband — Vincha PEMF
>>>>>>> Stashed changes
La vincha es el componente encargado de aplicar la estimulación electromagnética.

Incorpora dos bobinas rectangulares ubicadas detrás de las orejas y el microcontrolador encargado de gestionar el sistema.

Las bobinas generan campos magnéticos pulsados a partir de las señales eléctricas proporcionadas por el circuito de control.

El diseño mecánico contempla una estructura que permite alojar las bobinas y proteger los componentes electrónicos, manteniendo un formato portátil.

<<<<<<< Updated upstream
<p align="center">
  <img src="docs/headband.png" width="500" alt="N.E.R.A. Headband">
</p>

---

### 2. N.E.R.A. Watch — Sistema de monitoreo

=======
N.E.R.A. Headband

2. N.E.R.A. Watch — Sistema de monitoreo
>>>>>>> Stashed changes
El reloj es el componente destinado al monitoreo del usuario.

Su función dentro del proyecto es proporcionar información al sistema para permitir la gestión de la estimulación.

El reloj funciona de manera independiente de la vincha y cuenta con su propia alimentación eléctrica.

La comunicación entre el reloj y el microcontrolador de la vincha permite coordinar el funcionamiento de los distintos componentes del sistema.

<<<<<<< Updated upstream
<p align="center">
  <img src="docs/watch.png" width="400" alt="N.E.R.A. Watch">
</p>

---

### 3. Sistema PEMF

=======
N.E.R.A. Watch

3. Sistema PEMF
>>>>>>> Stashed changes
El sistema PEMF está compuesto principalmente por las bobinas y la electrónica necesaria para controlar su funcionamiento.

Las bobinas reciben señales eléctricas controladas por el sistema electrónico y generan campos electromagnéticos pulsados.

El sistema está diseñado para funcionar de forma no invasiva y se encuentra integrado físicamente en la vincha.

<<<<<<< Updated upstream
<p align="center">
  <img src="docs/coils.png" width="500" alt="Bobinas del sistema PEMF">
</p>

---

## Tecnología PEMF

PEMF (*Pulsed Electromagnetic Fields*) es una tecnología basada en la generación de campos electromagnéticos que varían mediante pulsos.
=======
Bobinas del sistema PEMF

Tecnología PEMF
PEMF (Pulsed Electromagnetic Fields) es una tecnología basada en la generación de campos electromagnéticos que varían mediante pulsos.
>>>>>>> Stashed changes

Su funcionamiento se relaciona con el principio de inducción electromagnética: cuando una corriente eléctrica circula por una bobina, genera un campo magnético. Al modificar esa corriente, el campo magnético también cambia.

La investigación sobre PEMF estudia la interacción de estos campos con los tejidos biológicos y sus posibles efectos en determinados procesos celulares.

Existen investigaciones sobre diferentes aplicaciones de esta tecnología, aunque sus efectos dependen de las características de la señal, la intensidad, la frecuencia y la aplicación estudiada.

En N.E.R.A., esta tecnología se utiliza como base para desarrollar un dispositivo orientado principalmente a la relajación y el manejo del estrés. Su eficacia para estos fines deberá evaluarse mediante pruebas específicas.

<<<<<<< Updated upstream
---

## Objetivos del proyecto

=======
Objetivos del proyecto
>>>>>>> Stashed changes
El objetivo principal de N.E.R.A. es desarrollar un dispositivo portátil que permita explorar la aplicación de campos electromagnéticos pulsados como herramienta de apoyo para la relajación.

Entre sus objetivos específicos se encuentran:

<<<<<<< Updated upstream
* Desarrollar un sistema PEMF compacto y portátil.
* Integrar la generación de campos electromagnéticos pulsados en una vincha.
* Implementar un sistema electrónico capaz de controlar el funcionamiento de las bobinas.
* Integrar un sistema de monitoreo mediante un reloj.
* Establecer comunicación entre el reloj y la electrónica de la vincha.
* Diseñar carcasas y soportes que permitan alojar y proteger los componentes.
* Fabricar prototipos mediante impresión 3D.
* Investigar las posibilidades de aplicación de la tecnología PEMF en el bienestar y la relajación.

---

## Desarrollo tecnológico

El proyecto integra distintas áreas de la ingeniería y el desarrollo tecnológico.

| Área            | Desarrollo                                                         |
| --------------- | ------------------------------------------------------------------ |
| Electrónica     | Diseño de circuitos, selección de componentes y desarrollo de PCB. |
| Firmware        | Programación del microcontrolador y control del sistema.           |
| Diseño mecánico | Modelado 3D de carcasas, soportes y alojamientos para las bobinas. |
| Impresión 3D    | Fabricación de piezas para el ensamblaje del dispositivo.          |
| Integración     | Conexión y coordinación de los diferentes módulos del sistema.     |

### Electrónica

=======
Desarrollar un sistema PEMF compacto y portátil.
Integrar la generación de campos electromagnéticos pulsados en una vincha.
Implementar un sistema electrónico capaz de controlar el funcionamiento de las bobinas.
Integrar un sistema de monitoreo mediante un reloj.
Establecer comunicación entre el reloj y la electrónica de la vincha.
Diseñar carcasas y soportes que permitan alojar y proteger los componentes.
Fabricar prototipos mediante impresión 3D.
Investigar las posibilidades de aplicación de la tecnología PEMF en el bienestar y la relajación.
Desarrollo tecnológico
El proyecto integra distintas áreas de la ingeniería y el desarrollo tecnológico.

Área	Desarrollo
Electrónica	Diseño de circuitos, selección de componentes y desarrollo de PCB.
Firmware	Programación del microcontrolador y control del sistema.
Diseño mecánico	Modelado 3D de carcasas, soportes y alojamientos para las bobinas.
Impresión 3D	Fabricación de piezas para el ensamblaje del dispositivo.
Integración	Conexión y coordinación de los diferentes módulos del sistema.
Electrónica
>>>>>>> Stashed changes
El diseño electrónico contempla una placa de circuito impreso que integra el microcontrolador y los componentes necesarios para controlar el sistema.

La distribución de los componentes y las conexiones se plantea teniendo en cuenta el tamaño del dispositivo, la alimentación y la disposición de las bobinas.

<<<<<<< Updated upstream
<p align="center">
  <img src="docs/pcb.png" width="500" alt="PCB del sistema N.E.R.A.">
</p>

### Diseño mecánico

=======
PCB del sistema N.E.R.A.

Diseño mecánico
>>>>>>> Stashed changes
El diseño mecánico se centra en desarrollar una estructura que permita integrar los componentes de manera compacta y segura.

Se trabaja en el modelado de las carcasas de la placa electrónica, los alojamientos de las bobinas, separadores y soportes necesarios para el ensamblaje.

Las piezas se diseñan considerando su fabricación mediante impresión 3D.

<<<<<<< Updated upstream
<p align="center">
  <img src="docs/mechanical.png" width="500" alt="Diseño mecánico de N.E.R.A.">
</p>

---

## Tecnologías y herramientas

Las principales tecnologías y herramientas utilizadas en el desarrollo incluyen:

* **ESP32:** microcontrolador utilizado para gestionar el sistema electrónico.
* **C/C++:** programación del firmware.
* **Diseño de PCB:** desarrollo de circuitos electrónicos.
* **Blender:** modelado tridimensional de componentes y carcasas.
* **Impresión 3D:** fabricación de piezas mecánicas y prototipos.
* **GitHub:** almacenamiento, organización y seguimiento del desarrollo del proyecto.

---

## Estructura del repositorio

El repositorio está organizado para separar los distintos componentes del proyecto y facilitar su desarrollo.

| Directorio    | Contenido                                                |
| ------------- | -------------------------------------------------------- |
| `docs/`       | Documentación técnica, imágenes e informes del proyecto. |
| `firmware/`   | Código fuente y programas del microcontrolador.          |
| `hardware/`   | Esquemáticos, circuitos y diseños de PCB.                |
| `mechanical/` | Modelos 3D, carcasas, soportes y diseños mecánicos.      |

---

## Estado del proyecto

**En desarrollo.**
=======
Diseño mecánico de N.E.R.A.

Tecnologías y herramientas
Las principales tecnologías y herramientas utilizadas en el desarrollo incluyen:

ESP32: microcontrolador utilizado para gestionar el sistema electrónico.
C/C++: programación del firmware.
Diseño de PCB: desarrollo de circuitos electrónicos.
Blender: modelado tridimensional de componentes y carcasas.
Impresión 3D: fabricación de piezas mecánicas y prototipos.
GitHub: almacenamiento, organización y seguimiento del desarrollo del proyecto.
Estructura del repositorio
El repositorio está organizado para separar los distintos componentes del proyecto y facilitar su desarrollo.

Directorio	Contenido
docs/	Documentación técnica, imágenes e informes del proyecto.
firmware/	Código fuente y programas del microcontrolador.
hardware/	Esquemáticos, circuitos y diseños de PCB.
mechanical/	Modelos 3D, carcasas, soportes y diseños mecánicos.
Estado del proyecto
En desarrollo.
>>>>>>> Stashed changes

N.E.R.A. se encuentra en una etapa de diseño, desarrollo e integración de sus componentes electrónicos y mecánicos.

El proyecto contempla el desarrollo de prototipos, la integración de los sistemas y la realización de pruebas para evaluar su funcionamiento.

Las funcionalidades y aplicaciones previstas se irán documentando a medida que sean implementadas y validadas.

<<<<<<< Updated upstream
---

## Aplicaciones previstas

=======
Aplicaciones previstas
>>>>>>> Stashed changes
El proyecto se centra principalmente en investigar el uso de la tecnología PEMF como posible herramienta complementaria para favorecer la relajación y el bienestar.

También se consideran posibles aplicaciones relacionadas con el descanso y la recuperación física, incluidos contextos deportivos.

Estas aplicaciones forman parte de los objetivos de investigación y desarrollo del proyecto. N.E.R.A. no está validado como tratamiento para trastornos de salud mental ni sustituye la atención profesional.

<<<<<<< Updated upstream
---

## Institución

**Escuela de Educación Secundaria Técnica N.º 7 «Taller Regional Quilmes»**

Buenos Aires, Argentina.

**Proyecto:** N.E.R.A.
**Significado:** Neuro Estimulación de Relajación Asistida
**Área:** Aviónica y desarrollo tecnológico.

---

<p align="center">
  <strong>N.E.R.A. — Neuro Estimulación de Relajación Asistida.</strong>
</p>
=======
Institución
Escuela de Educación Secundaria Técnica N.º 7 «Taller Regional Quilmes»

Buenos Aires, Argentina.

Proyecto: N.E.R.A. Significado: Neuro Estimulación de Relajación Asistida Área: Aviónica y desarrollo tecnológico.

N.E.R.A. — Neuro Estimulación de Relajación Asistida.
>>>>>>> Stashed changes
