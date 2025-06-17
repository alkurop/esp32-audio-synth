#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <driver/pulse_cnt.h>
#include <functional>
#include <cstdint>

namespace ui
{

    /**
     * @brief Configuration for the Rotary encoder pins, ID, and behavior
     */
    struct RotaryConfig
    {
        uint8_t id = 0;     ///< Identifier for this encoder instance
        gpio_num_t pin_clk; ///< GPIO number for encoder CLK pin
        gpio_num_t pin_dt;  ///< GPIO number for encoder DT pin

        int16_t minValue = 0;   ///< Minimum allowed encoder value
        int16_t maxValue = 127; ///< Maximum allowed encoder value
        uint8_t increment = 1;
        uint16_t glitchFilterNs = 100;
        bool wrapAround = false;
    };

    /**
     * @brief Wrapper for an EC11-style rotary encoder using PCNT
     */
    class Rotary
    {
    public:
        RotaryConfig config;
        QueueHandle_t eventQueue = nullptr;

        using RotaryCallback = std::function<void(uint8_t id, int16_t newValue)>;

        /**
         * @brief Construct a new Rotary encoder handle
         * @param config  Pin, ID, and behavior configuration
         */
        explicit Rotary(const RotaryConfig &config);

        /**
         * @brief Initialize the encoder and register a callback for value changes
         * @param cb  Function called with (id, newValue) when position changes
         */
        void init(const RotaryCallback &cb);

        /**
         * @brief Set the minimum and maximum allowed value at runtime
         * @param minValue  Lower bound for the encoder value
         *          * @param maxValue  Upper bound for the encoder value

         */
        inline void setRange(int16_t minValue, int16_t maxValue)
        {
            config.minValue = minValue;
            config.maxValue = maxValue;
        }

        inline void setIncrement(uint8_t increment)
        {
            config.increment = increment;
        }

        /**
         * @brief Force the current position (will invoke callback)
         * @param value  New position to set
         */
        inline void setPosition(int16_t value, bool send = false)
        {
            currentPosition = value;
            if (send && callback)
            {
                callback(config.id, currentPosition);
            }
        }

        /**
         * @brief Force the current position (will invoke callback)
         * @param value  New position to set
         */
        inline void setWrapAround(bool wrapAround)
        {
            config.wrapAround = wrapAround;
        }

    private:
        RotaryCallback callback = nullptr;
        int16_t currentPosition = 0;

        pcnt_unit_handle_t unit = nullptr;
        void initGPIOInterrupt();
        void initPCNT();
        void startTask();
        void processEvents();
    };

} // namespace ui
