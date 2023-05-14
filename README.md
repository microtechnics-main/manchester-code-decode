# Manchester decoding

<img src="https://microtechnics.ru/wp-content/uploads/2021/08/dekodirovanie-manchesterskogo-koda-1.jpg" width="400">

Decoding process of [previously generated](https://github.com/microtechnics-main/manchester-code-encode) Manchester code.

Used software and hardware:
- MCU: STM32F103C8
- IDE: IAR Embedded Workbench
- STM32CubeMx
- HAL Driver

Encoding process is based on one Timer module (TIM2) and one GPIO for signal generating (PA3). Decoding part uses GPIO pin configured as EXTI line (PA4) and the same Timer (TIM2). A detailed description is again available on my site - [post link](https://microtechnics.ru/manchesterskij-kod-chast-2-dekodirovanie-dannyh/).

Some resulting debug info:

<img src="https://microtechnics.ru/wp-content/uploads/2021/08/primer-programmy-dekodirovaniya.jpg" width="400">
