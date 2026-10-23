/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 四路流水灯 + 按键改变方向（HAL 库 / MDK-ARM）
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* ==========================================================================
 * 一、接线表（只改这里就能换引脚）
 * --------------------------------------------------------------------------
 * WIRING = 1 : 面包板上实际接线 —— PB12 蓝 / PB13 黄 / PB14 绿 / PB15 红
 * WIRING = 0 : 早期 CubeMX 引脚图 —— PB8 蓝 / PB9 绿 / PA1 黄 / PA2 红 / PC13 LED
 *
 * LED 接法：长脚(正极) -- 限流电阻 -- GPIO ；短脚(负极) -- 面包板负极轨(GND)
 *            => 高电平点亮（GPIO_PIN_SET = 亮）
 * 按键接法：PA0 -- 按键 -- GND ，内部上拉，按下读到低电平
 * ========================================================================== */
#define WIRING                 1

#if (WIRING == 1)
  #define LED_BLUE_PORT        GPIOB
  #define LED_BLUE_PIN         GPIO_PIN_12
  #define LED_YELLOW_PORT      GPIOB
  #define LED_YELLOW_PIN       GPIO_PIN_13
  #define LED_GREEN_PORT       GPIOB
  #define LED_GREEN_PIN        GPIO_PIN_14
  #define LED_RED_PORT         GPIOB
  #define LED_RED_PIN          GPIO_PIN_15
#else
  #define LED_BLUE_PORT        GPIOB
  #define LED_BLUE_PIN         GPIO_PIN_8
  #define LED_YELLOW_PORT      GPIOA
  #define LED_YELLOW_PIN       GPIO_PIN_1
  #define LED_GREEN_PORT       GPIOB
  #define LED_GREEN_PIN        GPIO_PIN_9
  #define LED_RED_PORT         GPIOA
  #define LED_RED_PIN          GPIO_PIN_2
#endif

#define BTN_PORT               GPIOA
#define BTN_PIN                GPIO_PIN_0   /* main.h 里 CubeMX 生成的 BTN_Pin 是同一根线，这里不重复使用 */

#define LED_COUNT              4U      /* LED 个数 */
#define STEP_MS                200U    /* 每走一步的间隔(ms)，流水速度就靠它 */
#define DEBOUNCE_MS            20U     /* 按键消抖时间(ms) */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* 每颗灯的 端口 + 引脚，数组顺序就是流水顺序 */
static GPIO_TypeDef *const led_port[LED_COUNT] = {
  LED_BLUE_PORT, LED_YELLOW_PORT, LED_GREEN_PORT, LED_RED_PORT
};
static const uint16_t led_pin[LED_COUNT] = {
  LED_BLUE_PIN, LED_YELLOW_PIN, LED_GREEN_PIN, LED_RED_PIN
};

static int8_t  s_dir      = 1;   /* +1 正向(蓝→黄→绿→红)，-1 反向 */
static int8_t  s_index    = 0;   /* 当前亮的是第几颗 */
static uint32_t s_tick    = 0;   /* 上一次换灯的时刻 */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
static void leds_all_off(void);
static void led_on(int8_t i);
static void chaser_step(void);
static void button_scan(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* 全部熄灭 */
static void leds_all_off(void)
{
  for (uint8_t i = 0; i < LED_COUNT; i++)
  {
    HAL_GPIO_WritePin(led_port[i], led_pin[i], GPIO_PIN_RESET);
  }
}

/* 点亮第 i 颗（清场由 leds_all_off 负责） */
static void led_on(int8_t i)
{
  HAL_GPIO_WritePin(led_port[i], led_pin[i], GPIO_PIN_SET);
}

/* 走一步：先全灭，再按下标把灯往当前方向挪一格 */
static void chaser_step(void)
{
  leds_all_off();

  s_index += s_dir;
  if (s_index >= (int8_t)LED_COUNT) s_index = 0;                    /* 正向到头，回到第 1 颗 */
  if (s_index < 0)                  s_index = (int8_t)LED_COUNT - 1; /* 反向到头，回到最后 1 颗 */

  led_on(s_index);
}

/* 按键扫描：20ms 消抖，只在按下那一刻翻转一次方向（非阻塞，不耽误流水） */
static void button_scan(void)
{
  static uint8_t  stable   = GPIO_PIN_SET;   /* 消抖后的电平，默认松开 = 高 */
  static uint8_t  raw_last = GPIO_PIN_SET;   /* 上一次读到的原始电平 */
  static uint32_t t_change = 0;
  uint8_t raw = (uint8_t)HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN);

  if (raw != raw_last)                       /* 电平跳变，重新计时 */
  {
    raw_last = raw;
    t_change = HAL_GetTick();
  }

  if ((HAL_GetTick() - t_change) >= DEBOUNCE_MS && raw != stable)
  {
    stable = raw;                            /* 电平稳住 20ms 才认账 */
    if (stable == GPIO_PIN_RESET)            /* 下降沿 = 按下 */
    {
      s_dir = (int8_t)(-s_dir);              /* 正反向切换 */
    }
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  leds_all_off();
  s_index = 0;
  led_on(s_index);                 /* 开机先点亮第一颗（蓝） */
  s_tick = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    button_scan();                 /* 每圈都扫，按键一按立刻响应，不会被延时卡住 */

    if ((HAL_GetTick() - s_tick) >= STEP_MS)
    {
      s_tick = HAL_GetTick();
      chaser_step();
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* 先输出低电平（灯灭），避免上电瞬间乱闪 */
  HAL_GPIO_WritePin(LED_BLUE_PORT,   LED_BLUE_PIN,   GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_YELLOW_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_GREEN_PORT,  LED_GREEN_PIN,  GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_RED_PORT,    LED_RED_PIN,    GPIO_PIN_RESET);

  /* LED 推挽输出（WIRING=0 时灯分散在 A/B 两个端口，所以逐个初始化） */
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = LED_BLUE_PIN;
  HAL_GPIO_Init(LED_BLUE_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LED_YELLOW_PIN;
  HAL_GPIO_Init(LED_YELLOW_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LED_GREEN_PIN;
  HAL_GPIO_Init(LED_GREEN_PORT, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = LED_RED_PIN;
  HAL_GPIO_Init(LED_RED_PORT, &GPIO_InitStruct);

  /* 按键 PA0：上拉输入，按下 = 低电平 */
  GPIO_InitStruct.Pin  = BTN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_PORT, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
