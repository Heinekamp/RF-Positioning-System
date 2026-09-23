/**
 * @file flash.h
 * @author Max Heinekamp
 * @brief Handles the flash memory for the ESP32.
 * @version 0.1
 * @date 28-04-2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <preferences.h>
#include "config.h"

/**
 * @brief This class handles access to the flash memory of the ESP32. It is also used to initialize the flash memory with default values.
 * 
 */
class FLASH {
    public:
        /**
         * @brief Construct a new FLASH object
         * 
         */
        FLASH();

        /**
         * @brief Initialize the flash memory and set default values if not already set.
         * 
         */
        void initFlashMemory();


        Preferences preferences;  // Preferences object for flash memory access
        
};