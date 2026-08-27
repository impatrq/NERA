#include "display.h"
#include "esp_log.h"
#include "pantalla_principal.h"

void app_main(void)
{
    ESP_ERROR_CHECK(nera_display_inicializar());
    ESP_ERROR_CHECK(nera_pantalla_principal_mostrar());
}
