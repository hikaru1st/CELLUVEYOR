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
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
CAN_FilterTypeDef CanFilter;

uint16_t TxRegID = 0x501;
uint16_t RxRegID = 0x502;
uint16_t TxDataID = 0x503;
uint16_t TxStateID = 0x504;
uint16_t RxVerifyID = 0x505;

uint32_t TxMailbox;
uint8_t TxData[8];
uint8_t RxData[8];
uint8_t START_CAN = 0;
uint8_t TEST = 0, preTEST = 0;
uint16_t TT_Receive = 0, Receive = 0;
uint8_t i = 0;

int8_t rx_buffer[12];
int16_t rx_X = 0, rx_Y = 0, rx_angle = 0;
int16_t *p_X, *p_Y, *p_angle;
int8_t Path[4];
int8_t _Path[4];
uint8_t leng_path =0;
uint8_t State = 0, preState = 0, Mode = 0;                     
uint8_t* p_State; // = (uint8_t*)0x1234;
		
uint8_t TT_RX_DATA = 0;

int16_t pos = 0, pre_pos = 0;
int16_t dir = 0 ; 
int16_t vel = 0, pre_vel = 0;
int16_t avg_vel = 95;
uint8_t number = 15;
uint16_t des_ctl = 0, pre_des_ctl = 0;

int8_t Trung = 0;
									
struct NearAddr
{
	int8_t X;
	int8_t Y;
	int16_t dir;
	int8_t stt;
};
struct Address
{
	int8_t X;
	int8_t Y;
	int8_t stt;
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */
//Ham gui tin nhan CAN
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

//Ham ngat nhan tin nhan CAN
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{ 

	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK)
	{			
		if( RxHeader.StdId ==  RxRegID)
		{
			number = RxData[0];
		}
		
		if( RxHeader.StdId ==  RxVerifyID)
		{
			number = RxData[0];
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		TT_RX_DATA = 1;
		// Process received data here
		HAL_UART_Receive_IT(&huart1, (uint8_t*)rx_buffer, sizeof(rx_buffer));
		uint8_t* p_rxX = (uint8_t*)&rx_X;
		uint8_t* p_rxY = (uint8_t*)&rx_Y;
		int8_t* p_rxAngle = (int8_t*)&rx_angle;
		
		p_State = &State;
		*p_State = (int)rx_buffer[0];
		Mode = (int)rx_buffer[1];
		
		for(int i = 0; i < 2; i++)
		{
			*(p_rxAngle + i) = rx_buffer[i+2];		
		}
		p_angle = &rx_angle;
		for(int i = 0; i < 2; i++)
		{
			*(p_rxX + i) = rx_buffer[i + 4];		
		}
		p_X = &rx_X;
		for(int i = 0; i < 2; i++)
		{
			*(p_rxY + i) = rx_buffer[i + 6];		
		}
		p_Y = &rx_Y;
		for(int i = 0; i < 4; i++)
		{
			Path[i] = (int)rx_buffer[i+8];
		}
		
		pos = 0;
		if(Mode == 3)
		{
			des_ctl = rx_X;
			if(des_ctl == 1) //control Postion
			{
				pos = rx_angle;
				vel = 0;
			}
			else if(des_ctl == 2)
			{
				vel = rx_angle;
				pos = 0;
			}
		}	
	}
}


void CreatAddr(struct Address address[])
{
	
	address[0].X = -1;
	address[0].Y = -1;
	address[0].stt = 0;
	
	address[1].X = 0;
	address[1].Y = 0;
	address[1].stt = 1;
		
	address[2].X = 0;
	address[2].Y = 2;
	address[2].stt = 2;
	
	address[3].X = 0;
	address[3].Y = 4;
	address[3].stt = 3;
	
	address[4].X = 1;
	address[4].Y = 1;
	address[4].stt = 4;
	
	address[5].X = 1;
	address[5].Y = 3;
	address[5].stt = 5;
	
	address[6].X = 2;
	address[6].Y = 0;
	address[6].stt = 6;
	
	address[7].X = 2;
	address[7].Y = 2;
	address[7].stt = 7;
	
	address[8].X = 2;
	address[8].Y = 4;
	address[8].stt = 8;
	
	address[9].X = 3;
	address[9].Y = 1;
	address[9].stt = 9;
	
	address[10].X = 3;
	address[10].Y = 3;
	address[10].stt = 10;
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_CAN_Start(&hcan);
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);
	
	
	HAL_NVIC_SetPriority(CAN_IT_RX_FIFO0_MSG_PENDING, 1, 0);
	HAL_NVIC_EnableIRQ(CAN_IT_RX_FIFO0_MSG_PENDING);
	HAL_UART_Receive_IT(&huart1,(uint8_t*)rx_buffer,sizeof(rx_buffer));
	
	struct NearAddr near_addr[6];
	for(int i = 0; i < 6; i++)
	{
		near_addr[i].X = 0;
		near_addr[i].Y = 0;
		near_addr[i].dir = 0;
		near_addr[i].stt = 0;
	}
	
	struct Address address[11];
	CreatAddr(address);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		// Gui Trang thai va Che do xuong Slave moi khi duoc cap nhat
		/* ---- Thuc hien viec gui tin nhan de xac nhan viec ket noi giao tiep CAN ---- */
		
		if(START_CAN == 0) 
		{
			for(int i = 1; i <= 10; i++)
			{
				do
				{
					TxData[0] = i;
					CAN_Send(TxRegID, 1);
				}
				while(number != i);
			}
			START_CAN = 1;
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		}
		
		/*----Thuc hien viec gui tin nhan de dieu khien dong co----*/
		else  //(START_CAN == 1)
		{	
			if(State != preState) //Gui trang thai va che do xuong moi slave
			{
				for(int i = 1; i <= 10; i++)
				{
					do
					{
						TxData[0] = i;
						TxData[1] = State;
						TxData[2] = Mode;
						CAN_Send(TxStateID, 3);
					}
					while(number != i);
					number = 0;
				}
				preState = State;
			}
			if(*p_State == 1) 
			{
				if(Mode == 3) // -------------------------------------- Che do TEST --------------------------------------------------
				{										
					if(des_ctl != pre_des_ctl || pos != pre_pos || vel != pre_vel)
					{
						for(int i = 1; i <= 10; i++)
						{
							do
							{
								TxData[0] = i;
								TxData[1] = pos >> 8;
								TxData[2] = pos;
								TxData[3] = dir >> 8;
								TxData[4] = dir;
								TxData[5] = vel >> 8;
								TxData[6] = vel;
								CAN_Send(TxDataID, 7);
							}
							while( number != i);	
							number = 0;
						}
					pre_des_ctl = des_ctl;
					pre_pos = pos;
					pre_vel = vel;
					}
				}
			
				else if(Mode == 2) // Che do MANUAL -------------------------------------------------------------
				{
					if(Path[0] > 0 && Path[0] <= 10)
					{
						for(int i = 0; i < 4; i++)
						{
							_Path[i] = Path[i];
						}
						//Tìm do dai cua duong di
						leng_path = 0;
						for(int i = 0; i < 4; i++)
						{
							if(_Path[i] > 0 && _Path[i] <=10)	leng_path ++;
						}
						
						///.................Gui data cho duong di chính..................//
						for(int i = 0; i < leng_path -1; i++)
						{ 
							switch(address[_Path[i+1]].X - address[_Path[i]].X)
							{
								case 0:
									if(address[_Path[i+1]].Y - address[_Path[i]].Y == 2)
									{
										dir = 0;
										vel = avg_vel;
									}
									else if(address[_Path[i+1]].Y - address[_Path[i]].Y == -2)
									{
										dir = 180;
										vel = avg_vel;
									}
									break;							
								case 1:
									if(address[_Path[i+1]].Y - address[_Path[i]].Y == 1)
									{
										dir = 60;
										vel = avg_vel;
									}
									else if (address[_Path[i+1]].Y - address[_Path[i]].Y == -1)
									{
										dir = 120;
										vel = avg_vel;
									}
									break;
								case -1:
									if(address[_Path[i+1]].Y - address[_Path[i]].Y == 1)
									{
										dir = -60;
										vel = avg_vel;
									}
									else if(address[_Path[i+1]].Y - address[_Path[i]].Y == -1)
									{
										dir = -120;
										vel = avg_vel;
									}
									break;	
									
								default: 
									break;
							}							
							do
							{
								TxData[0] = _Path[i];
								TxData[1] = pos >> 8;
								TxData[2] = pos;
								TxData[3] = dir >> 8;
								TxData[4] = dir;
								TxData[5] = vel >> 8;
								TxData[6] = vel;
								CAN_Send(TxDataID, 7);
							}	while(number != _Path[i]);
								number = 0;
						}		//..........Ket thuc gui data cho duong di chính..............//
						
						// Tim cac Module xung quanh duong di và huong di cua nó.
						for(int i = 1; i < leng_path; i++)
						{ 
								TT_Receive = _Path[i];
							for(int k = 0; k < 6; k++)
							{
								near_addr[k].X = 0;
								near_addr[k].Y = 0;
								near_addr[k].dir = 0;
								near_addr[k].stt = 0;
							}
							
							
							near_addr[0].X = address[_Path[i]].X -1;  //  Path[3] address[Path[1]][0]  near_addr[5].stt  near_addr[5].dir  near_addr[3].dir
							near_addr[0].Y = address[_Path[i]].Y -1;
							near_addr[0].dir = 60;
							
							near_addr[1].X = address[_Path[i]].X -1;
							near_addr[1].Y = address[_Path[i]].Y +1;
							near_addr[1].dir = 120;
							
							near_addr[2].X = address[_Path[i]].X;
							near_addr[2].Y = address[_Path[i]].Y -2;
							near_addr[2].dir = 0;
							
							near_addr[3].X = address[_Path[i]].X;
							near_addr[3].Y = address[_Path[i]].Y +2;
							near_addr[3].dir = 180;
							
							near_addr[4].X = address[_Path[i]].X +1;
							near_addr[4].Y = address[_Path[i]].Y -1;
							near_addr[4].dir = -60;
							
							near_addr[5].X = address[_Path[i]].X +1;
							near_addr[5].Y = address[_Path[i]].Y +1;
							near_addr[5].dir = -120;
							
							Receive = 2;
							
							/*         SAI TU DAY <Khong thoat ra khoi vong lap duoc>           */
							
							//Tao STT cho cac module xung quanh
							for(int k = 0; k < 6; k++)   
							{
								Trung = 0;
								for(int j = 1; j <= 10; j++)
								{
									
									if((near_addr[k].X == address[j].X) && (near_addr[k].Y == address[j].Y))
									{
										near_addr[k].stt = address[j].stt;
										Receive = 3;
									}
									else 
									{
										Trung ++;
										Receive = 4;
									}
								}
								if(Trung == 10)
								{							
									near_addr[k].X = -1;
									near_addr[k].Y = -1;
									near_addr[k].dir = -1;
									near_addr[k].stt = -1;							
								}							
							} //..........Ket thuc tao stt cho cac module..............//
							
							//.....Kiem tra cac module xung quanh da tao ra có trùng voi duong di ban dau khong. Có thì xóa
							for(int k = 0; k < 6; k++)
							{
								for(int j = 0; j < 4; j++)
								{
									if(near_addr[k].stt == _Path[j])
									{
										near_addr[k].X = -1;
										near_addr[k].Y = -1;
										near_addr[k].dir = -1;
										near_addr[k].stt = -1;
									}
								}			
							} //.....Ket thuc xoa cac Module trung voi duong di chinh
							for(int k = 0; k < 6; k++)
							{
								if(near_addr[k].stt > 0 && near_addr[k].stt <= 10)
								{
									do
									{
										TxData[0] = near_addr[k].stt;
										TxData[1] = pos >> 8;
										TxData[2] = pos;
										TxData[3] = near_addr[k].dir >> 8;
										TxData[4] = near_addr[k].dir;
										TxData[5] = vel >> 8;
										TxData[6] = vel;
										CAN_Send(TxDataID, 7);
									} while( number != near_addr[k].stt);
									number = 0;
								}
							}
						} //..........Ket thuc gui data cho cac module xung quang duong di..............//
						
						while((*p_X != address[_Path[leng_path-1]].X || *p_Y != address[_Path[leng_path-1]].Y))
						{
							if (*p_State != 1 ) break;
							Receive ++;
		
						}
						
						for(int i = 1; i <= 10; i++)
						{
							if (*p_State != 1 ) break;
							vel = 0; pos = 0; dir = 0;
							do
								{
								TxData[0] = i;
								TxData[1] = pos >> 8;
								TxData[2] = pos;
								TxData[3] = dir >> 8;
								TxData[4] = dir;
								TxData[5] = vel >> 8;
								TxData[6] = vel;
								CAN_Send(TxDataID, 7);
							}	while(number != i);
							number =0;
						}
						HAL_Delay(1000);
						pos = 90 - (*p_angle);
						if(pos > 15 || pos < -15)
						{
							TxData[0] = _Path[leng_path - 1];
							TxData[1] = pos >> 8;
							TxData[2] = pos;
							TxData[3] = dir >> 8;
							TxData[4] = dir;
							TxData[5] = vel >> 8;
							TxData[6] = vel;
							CAN_Send(TxDataID, 7);
						}
						Receive = 4;
					} //..................................Ngoac cua If(Path[0] > 0) << DK de CT chi gui data 1 lan duy nhat>>...............
				}	//............Ngoac cua Mode == 2..........//
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
	/* ------------ Cau hinh bo loc CAN --------------*/
  CanFilter.FilterActivation = CAN_FILTER_ENABLE;
  CanFilter.FilterBank = 1; // which filter bank to use from the assigned ones
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
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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
