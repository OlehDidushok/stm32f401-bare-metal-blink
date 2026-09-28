#include "stm32f4xx.h"

#define LED_PIN 13U

static void delay(volatile uint32_t count) {
    while (count--) { }
}

/* Вихід push-pull: транзистори тягнуть лінію і вгору, і вниз. */
static void gpio_led_init_pushpull(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;                // RCC: подати тактування на порт GPIOC, інакше регістри порту не пишуться
    GPIOC->MODER &= ~(0x3U << (LED_PIN * 2));           // MODER: скинути обидва біти режиму PC13 (стан 00 = вхід)
    GPIOC->MODER |=  (0x1U << (LED_PIN * 2));           // MODER: 01 = загальний вихід (general purpose output)
    GPIOC->OTYPER &= ~(0x1U << LED_PIN);                // OTYPER: 0 = push-pull, вихід сам формує і високий, і низький рівень
    GPIOC->OSPEEDR &= ~(0x3U << (LED_PIN * 2));         // OSPEEDR: 00 = low speed, для LED швидкість фронтів не потрібна
    GPIOC->PUPDR &= ~(0x3U << (LED_PIN * 2));           // PUPDR: 00 = без підтяжок, у push-pull вони зайві
}

/* Вихід open-drain: транзистор тягне лінію лише вниз, вгору — підтяжка. */
static void gpio_led_init_opendrain(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;                // RCC: подати тактування на порт GPIOC
    GPIOC->MODER &= ~(0x3U << (LED_PIN * 2));           // MODER: скинути біти режиму PC13
    GPIOC->MODER |=  (0x1U << (LED_PIN * 2));           // MODER: 01 = загальний вихід
    GPIOC->OTYPER |=  (0x1U << LED_PIN);                // OTYPER: 1 = open-drain, вихід тільки притягує до GND
    GPIOC->OSPEEDR &= ~(0x3U << (LED_PIN * 2));         // OSPEEDR: 00 = low speed
    GPIOC->PUPDR &= ~(0x3U << (LED_PIN * 2));           // PUPDR: скинути біти підтяжки PC13
    GPIOC->PUPDR |=  (0x1U << (LED_PIN * 2));           // PUPDR: 01 = внутрішній pull-up, він підтягує лінію до VDD замість транзистора
}

int main(void) {
     gpio_led_init_pushpull();
    //  gpio_led_init_opendrain();

    while (1) {
        GPIOC->BSRR = (1U << LED_PIN);                  // BSRR: біти 0..15 ставлять пін у 1 (на Black Pill LED тут гасне — катод на піні)
        delay(1000000);
        GPIOC->BSRR = (1U << (LED_PIN + 16));           // BSRR: біти 16..31 скидають пін у 0, LED світиться
        delay(1000000);
    }
}
