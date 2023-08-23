/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define number 10	// STT cua module

#define sampletime 0.01 // Thoi gian lay mau = Chu ky PWM 
#define encoder_resolution  11 // Do phan giai encoder chua qua hop so
#define gear_ratio 35	
#define PI 2*acos(0.0)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
CAN_FilterTypeDef CanFilter;


//Cac bien cho CAN
uint16_t TxID = 0x502; // Ðia chi gui tin nhan di den Master
uint16_t RegID = 0x501; // Ðia chi nhan tin nhan dang ki
uint16_t DataID = 0x503; // Dia chi nhan tin nhan Data de dieu khien dong co
uint16_t StateID = 0x504; // Dia chi nhan tin nhan Trang thai và che do hoat d?ng
uint16_t TxVerifyID = 0x505;

uint32_t TxMailbox;
uint8_t TxData[8];  // Mang chua tin nhan gui di
uint8_t RxData[8];  // Mang chua tin nhan nhan duoc

uint8_t START_CAN = 0;  // Bien de bat dau gui data dieu khien
uint8_t State = 0, Mode = 0;
uint8_t TT_Receive = 0; // Cac bien trang thai

// Du lieu nhan duoc
int16_t pos, pre_pos, pos1; // Goc can xoay tai tam cua module
int16_t dir, pre_dir; // Huong can di chuyen cua hop
uint16_t vel, pre_vel; // Toc do khi di chuyen
uint16_t count_data = 0, pre_count =0;

//Cac bien cho Encoder và PWM
volatile short count; // Bien de luu gia tri Encoder doc duoc
short encoder[3];

// Cac bien PID Vi tri
float Pos[4];
float KP_P = 5.6;
float KI_P = 0.026;
float KD_P = 0.0156;
float error_P[3] ;
float error1_P[3];
float error2_P[3];
float pwm_output_P[3] ; 
float lastpwm_output_P[3];
float alpha_P;
float	beta_P;
float	gamma_P;

// Cac bien PID Van toc
float V[3];
float KP_V = 0.5;
float KI_V = 0.005;
float KD_V = 0.01;
int8_t cur_dir[3] = {1, 1, 1}; // Huong quay cua dong co khi doc encoder
float cur_speed[3];
float error_V[3] ;
float error1_V[3];
float error2_V[3];
float pwm_output_V[3] ; 
float lastpwm_output_V[3];
float alpha_V;
float	beta_V;
float	gamma_V;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/*-----Ham gui tin nhan CAN-----*/
void CAN_Send(uint16_t ID, uint8_t leng)
{
	// Thiet lap các thông so cho gói tin du lieu can gui di
  TxHeader.StdId =  ID  ;        // Dia chi ID cua goi tin can gui di
  TxHeader.RTR = CAN_RTR_DATA;      // Loai gói tin du lieu
  TxHeader.IDE = CAN_ID_STD;       // Kieu dia chi ID
  TxHeader.DLC = leng;                // Ðo dài du lieu can gui di
  // Thêm gói tin du lieu vào hàng doi truyen và truyen di trên mang CAN
  if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailbox) == HAL_OK)
  {
  }
	else
	{
		Error_Handler();
	}
  // Cho den khi gói tin du lieu duoc truyen hoàn tat
  while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) != 3)
  {
  }
}


/*-----Ham ngat nhan du lieu CAN và xu ly-----*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	TT_Receive = 3;
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
	{ 
		TT_Receive = 2;
		
		// Nhan tin nhan xac nhan da ket noi giao tiep CAN thanh cong
		if(RxHeader.StdId ==  RegID && RxData[0] == (int)number)
		{	
			TT_Receive = 1;
			TxData[0] = (int)number;
			CAN_Send(TxID, 1);
			START_CAN = 1;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		}	
		// Nhan tin nhan trang thai hoat dong và che do
		if(RxHeader.StdId ==  StateID && RxData[0] == (int)number)
		{
			State = RxData[1];
			Mode = RxData[2];
			
			TxData[0] = (int)number; //Gui tin nhan xac nhan 
			CAN_Send(TxVerifyID, 1);
		}	
		//Xu li tin data nhan duoc de dieu khien
		if(RxHeader.StdId ==  DataID && RxData[0] == (int)number)
		{
			//Xu lí bien du lieu doc duoc
			pos = RxData[1] << 8 | RxData[2];
			dir = RxData[3] << 8 | RxData[4];
			vel = RxData[5] << 8 | RxData[6];
			
			TxData[0] = (int)number; // Gui tin nhan xac nhan
			CAN_Send(TxVerifyID, 1);
			count_data ++;
		}
	}
}


/*-----Ham doc gia tri Encoder và xac dinh chieu quay-----*/
int readEncoder(uint8_t motor) // motor chi stt cua dong co. 0 1 2
{
	switch(motor)
	{
		case 0:
		count = __HAL_TIM_GET_COUNTER(&htim2);
//		__HAL_TIM_SetCounter(&htim2, 0);
		break;
		
		case 1:
		count = __HAL_TIM_GET_COUNTER(&htim3);
//		__HAL_TIM_SetCounter(&htim3, 0);
		break;
		
		case 2:
		count = __HAL_TIM_GET_COUNTER(&htim4);
//		__HAL_TIM_SetCounter(&htim4, 0);
		break;		
	}
	
	if((int)count > 32768) 
	{
		count = count - 65536;
	}
	
	return count;
}


/* -----Ham dieu khien dong co quay thuan-----V----*/
void Forward(uint8_t motor, float velocity) // vel
{
	switch(motor)
	{
		case 0:
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // chan In 1
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);	// chan In 2
			__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, velocity); // Chan Enable
			break;
		case 1:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET); // chan In 1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);	// chan In 2
			__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, velocity); // Chan Enable
			break;
		case 2:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // chan In 1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);	// chan In 2
			__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, velocity); // Chan Enable	
			break;
	}
}
/* -----Ham dieu khien dong co quay nghich-----V----*/
void Backward(uint8_t motor, float velocity) // vel
{
	switch(motor)
	{
		case 0:
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET); // chan In 1
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);	// chan In 2
			__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_3, velocity); // Chan Enable
			break;
		case 1:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); // chan In 1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);	// chan In 2
			__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, velocity); // Chan Enable
			break;
		case 2:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // chan In 1
			HAL_GPIO_WritePin(GPIOA , GPIO_PIN_5, GPIO_PIN_RESET);	// chan In 2
			__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, velocity); // Chan Enable	
			break;
	}
}
/* -----Ham dung dong co-----V----*/
void Stop(uint8_t motor)
{
	switch(motor)
	{
		case 0:
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // chan In 1
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);	// chan In 2
			__HAL_TIM_SetCounter(&htim2, 0);
			break;
		case 1:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET); // chan In 1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);	// chan In 2
			__HAL_TIM_SetCounter(&htim3, 0);
			break;
		case 2:
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // chan In 1
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);	// chan In 2
			__HAL_TIM_SetCounter(&htim4, 0);
			break;
	}
}

int Ctl_Pos(float desired_pos)
{
	alpha_P = 2*sampletime*KP_P+KI_P*sampletime*sampletime+2*KD_P;
	beta_P = KI_P*sampletime*sampletime-4*KD_P-2*KP_P*sampletime;
	gamma_P = 2*KD_P;

	do
	{
		if(State == 0) break;
		
		encoder[0] = readEncoder(0);
		Pos[0] = (float)encoder[0]*360/(4*gear_ratio*encoder_resolution);
		error_P[0] = desired_pos - Pos[0];
		
		pwm_output_P[0] = (alpha_P*error_P[0] + beta_P*error1_P[0] + gamma_P*error2_P[0] + 2*sampletime*lastpwm_output_P[0])/(2*sampletime);
		
		lastpwm_output_P[0] = pwm_output_P[0];
		error2_P[0]=error1_P[0];
		error1_P[0]=error_P[0];
		
		//Gioi han gia tri PWM trong khoang tu -170 den 170;
		if(pwm_output_P[0] > 50) pwm_output_P[0] = 50;
		else if(pwm_output_P[0] < -50) pwm_output_P[0] = -50; 

		
		for(int i = 0; i < 3; i++)
		{
			if(pwm_output_P[0] > 0) Forward(i, pwm_output_P[0]);
			else if(pwm_output_P[0] < 0) Backward(i, fabs(pwm_output_P[0]));
			else Stop(i);
		}
		
		HAL_Delay(10);
	}
		while(error_P[0] > 3 || error_P[0] < -3);
		for(int i = 0; i < 3; i++)
		{
			Stop(i);
		}
		

}

void ctl_Vel(float desired_dir, float desired_vel)
{
	alpha_V=2*sampletime*KP_V+KI_V*sampletime*sampletime+2*KD_V;
	beta_V=KI_V*sampletime*sampletime-4*KD_V-2*KP_V*sampletime;
	gamma_V=2*KD_V;
	
	float dirR = dir * PI /180;
	V[0]= vel*sin(dirR) ;
	V[1] = vel*((sqrt(3)/2)*cos(dirR) - sin(dirR)/2);
	V[2] = vel*(-(sqrt(3)/2)*cos(dirR) - sin(dirR)/2);

	for(int i = 0; i < 3; i++)
	{
		if(V[i] > 0) Forward(i, V[i]);
		else if(V[i] < 0) Backward(i, fabs(V[i]));
		else Stop(i);
	}
	
	/*------------------- PID VAN TOC------------------*/
//	do
//	{
//		for(int i = 0; i < 3; i++)
//		{
//			encoder[i] = readEncoder(i);
//			
//			cur_speed[i] = (encoder[i]*60/(2*sampletime*gear_ratio*encoder_resolution));			
//			error_V[i] = V[i] - cur_speed[i];
//			
//			pwm_output_V[i] = (alpha_V*error_V[i] + beta_V*error1_V[i] + gamma_V*error2_V[i] + 2*sampletime*lastpwm_output_V[i])/(2*sampletime);
//	
//			lastpwm_output_V[i] = pwm_output_V[i];
//			error2_V[i]=error1_V[i];
//			error1_V[i]=error_V[i];
//			
//			// Gioi han PWM Output
//			if(pwm_output_V[i]>170) pwm_output_V[i] = 170;
//			else if(pwm_output_V[i] <-170) pwm_output_V[i] = -170;
//		}
//		
//		for(int i = 0; i < 3; i++)
//		{
//			if(pwm_output_V[i] > 0) Forward(i, pwm_output_V[i]);
//			else if(pwm_output_V[i] < 0) Backward(i, fabs(pwm_output_V[i]));
//			else Stop(i);
//			
//		}
//		__HAL_TIM_SetCounter(&htim2, 0);
//		__HAL_TIM_SetCounter(&htim3, 0);
//		__HAL_TIM_SetCounter(&htim4, 0);
//		HAL_Delay(10);
//	} while(error_V[0] > 2 || error_V[0] < -2);
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_CAN_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); 															// Bat dau che do PWM cua Timer1 Channel 1
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); 															// Bat dau che do PWM cua Timer1 Channel 2
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3); 															// Bat dau che do PWM cua Timer1 Channel 3
	
	
	HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_2);							// Bat dau che do Read Encoder cua Timer2 - Channel 1&2
	HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_1 | TIM_CHANNEL_2);							// Bat dau che do Read Encoder cua Timer3 - Channel 1&2
	HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_1 | TIM_CHANNEL_2);							// Bat dau che do Read Encoder cua Timer4 - Channel 1&2
	
	if(HAL_CAN_Start(&hcan) != HAL_OK)
	{
		Error_Handler();
	}
	
	if(HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		Error_Handler();
	}
	
	HAL_NVIC_SetPriority(CAN_IT_RX_FIFO0_MSG_PENDING, 0, 0);
	HAL_NVIC_EnableIRQ(CAN_IT_RX_FIFO0_MSG_PENDING);
	
	Stop(0);
	Stop(1);
	Stop(2);
	

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(START_CAN == 1)
		{
			if(State == 1)
			{
				/*------- Chuong Trinh TEST--------*/
				if(Mode == 3)
				{
					if(pos != pre_pos)
					{
						pos1 = (float)70/29*pos; // pos: goc can xoay cua module. pos1: goc xoay cua banh xe
						Ctl_Pos(pos1);
						pre_pos = pos;
						pre_vel = vel;
					}
					else if(vel != pre_vel)
					{
						if(vel > 0)
						{
							for(int i = 0; i < 3; i++)
							{
								Forward(i, vel);
							}
						}
						else if(vel < 0)
						{
							for(int i = 0; i < 3; i++)
							{
								Backward(i, vel);
							}
						}
						else if(vel == 0)
						{
							for(int i = 0; i < 3; i++)
							{
								Stop(i);
							}
						}
						pre_pos = pos;
						pre_vel = vel;
					}				
				}			
				/*--- Chuong trinh dieu khien chinh (Mode = 1 or Mode = 2) ---*/
				else if(Mode == 2)
				{
					if(count_data != pre_count)
					{
						if(pos != 0)
						{
							// DK Vitri
			
							pos1 = (float)70/29*pos; // pos: goc can xoay cua module. pos1: goc xoay cua banh xe
							Ctl_Pos(pos1);
							
							pre_pos = pos;
						}				
							// DK Van toc
						else
						{
							for(int i = 0; i < 3; i++)
							{
								Stop(i);
							}
							
							ctl_Vel(dir, vel); //vel: Van toc trung binh module
							
							pre_dir = dir;
							pre_vel = vel;
						}
						pre_count = count_data;	
					}						
				}
			}
			else //State = 0 : Stop ALL
			{
				for(int k = 0; k < 3; k++)
				{
					Stop(k);						
				}	
			}
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_4TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = ENABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

	/* CAU HINH BO LOC CAN----------------------*/
  CanFilter.FilterActivation = CAN_FILTER_ENABLE;
  CanFilter.FilterBank = 1; 
  CanFilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  CanFilter.FilterIdHigh = 0x0000;
  CanFilter.FilterIdLow = 0x0000;
  CanFilter.FilterMaskIdHigh = 0x0000;
  CanFilter.FilterMaskIdLow = 0x0000;
  CanFilter.FilterMode = CAN_FILTERMODE_IDMASK;
  CanFilter.FilterScale = CAN_FILTERSCALE_32BIT;
  CanFilter.SlaveStartFilterBank = 1;
  if(HAL_CAN_ConfigFilter(&hcan, &CanFilter) != HAL_OK) 
  {
	  Error_Handler();
  }
  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 2117;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 169;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA2 PA3 PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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

#ifdef  USE_FULL_ASSERT
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
