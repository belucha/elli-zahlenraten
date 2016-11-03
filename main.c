/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.32 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information

Licensor:                 SEGGER Software GmbH
Licensed to:              ARM Ltd, 110 Fulbourn Road, CB1 9NJ Cambridge, UK
Licensed SEGGER software: emWin
License number:           GUI-00181
License model:            LES-SLA-20007, Agreement, effective since October 1st 2011 
Licensed product:         MDK-ARM Professional
Licensed platform:        ARM7/9, Cortex-M/R4
Licensed number of seats: -
----------------------------------------------------------------------
File        : main.c
Purpose     : Main program Template
---------------------------END-OF-HEADER------------------------------
*/

#include "stm32f4xx_hal.h"
#include "RTE_Components.h"
#include "GUI.h"
#include "Board_Buttons.h"
#include <stdbool.h>

#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif

/* System Clock Configuration */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the
     device is clocked below the maximum system frequency (see datasheet). */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

void waitForButton() {
	while (Buttons_GetState()==0)
		__nop();
	while (Buttons_GetState()!=0)
		__nop();
}

unsigned getDigit(unsigned x, unsigned y, bool tens)
{
	unsigned number=0;
	for (;;) {
		GUI_SetFont(GUI_FONT_D24X32);
		GUI_DispDecAt(tens?number*10:number, x, y, tens?2:1);
		unsigned start = os_time;
		while ((os_time-start)<(number==0?2000:750))
			if (Buttons_GetState())
			{
				while (Buttons_GetState()==1)
					__nop();
				return number;
			}
		number=(number+1)%10;
	}	
}

unsigned getNumber(unsigned versuch, unsigned richtig, unsigned letzteZahl)
{
	GUI_SetBkColor(GUI_BLACK);
	GUI_Clear();

	int xSize = LCD_GetXSize();
	int ySize = LCD_GetYSize();

	GUI_SetFont(&GUI_FontComic24B_1);
	GUI_SetColor(GUI_CYAN);
	char buffer[64];
	snprintf(buffer, sizeof(buffer), "%u. Versuch!", versuch);
	GUI_DispStringHCenterAt(buffer,   xSize / 2, 20);
	GUI_SetColor(GUI_RED);
	GUI_DispStringHCenterAt("Heute ist Elektrotag!", xSize / 2, ySize - 40);
		
	int xPos = xSize / 2;
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_NORMAL);
	GUI_SetFont(GUI_FONT_20F_ASCII);
	GUI_DispStringHCenterAt(
		"Druecke die blaue Taste\n"
		"um anzuhalten!\n"
		, xPos, 60);

	if (versuch>1)
	{		
		snprintf(buffer, sizeof(buffer), "Ups %u war zu %s!\nNoch mal!\nDu schaffst das!", letzteZahl, letzteZahl<richtig?"klein":"gross");
		GUI_SetColor(GUI_RED);
		GUI_DispStringHCenterAt(buffer, xPos, 120);
	}
	unsigned tens = getDigit(60, 220, true);
	GUI_DispStringHCenterAt("+", 130, 220);
	unsigned one = getDigit(150, 220, false);	
	return tens*10+one;
}

typedef struct {
			unsigned limit;
			char* text;
} SBewertung;

/*********************************************************************
*
*       Main
*/
int main (void) {
  int xPos, xSize, ySize;
	
  HAL_Init();                           /* Initialize the HAL Library */
  SystemClock_Config();                 /* Configure the System Clock */

	RCC->AHB2ENR|=RCC_AHB2ENR_RNGEN;
	RNG->CR|=RNG_CR_RNGEN;
	
	Buttons_Initialize();
	
  GUI_Init();

  GUI_SetBkColor(GUI_RED);
  GUI_Clear();
  GUI_Delay(200);
  GUI_SetBkColor(GUI_GREEN);
  GUI_Clear();
  GUI_Delay(200);
  GUI_SetBkColor(GUI_BLUE);
  GUI_Clear();
  GUI_Delay(200);


	while (1) 
	{
		GUI_SetBkColor(GUI_BLACK);
		GUI_Clear();

		xSize = LCD_GetXSize();
		ySize = LCD_GetYSize();

		GUI_SetFont(&GUI_FontComic24B_1);
		GUI_SetColor(GUI_CYAN);
		GUI_DispStringHCenterAt("Tach Elli, Hallo 3b!",   xSize / 2, 20);
		GUI_SetColor(GUI_RED);
		GUI_DispStringHCenterAt("Heute ist Elektrotag!", xSize / 2, ySize - 40);
		
		unsigned number = 1+(RNG->DR%100);
		
		xPos = xSize / 2;
		GUI_SetColor(GUI_WHITE);
		GUI_SetTextMode(GUI_TM_NORMAL);
		GUI_SetFont(GUI_FONT_20F_ASCII);
		GUI_DispStringHCenterAt(
			"Wir spielen Zahlenraten!\n"
			"Ich habe mir eine Zahl\n"
			"zwischen 0 und 100\n"
			"ausgedacht.\n"
			"Du sollst raten!\n"
			"\n"
			"Zum Starten blaue \n"
			"Taste drücken!\n"
			, xPos, 60);
	
		waitForButton();
		
		unsigned wahl=1000;
		int versuche;
		for (versuche=1; number!=wahl; versuche++)
			wahl=getNumber(versuche, number, wahl);
			
		// geschafft
		GUI_SetBkColor(GUI_WHITE);
		GUI_Clear();

		GUI_SetFont(&GUI_FontComic24B_1);
		GUI_SetColor(GUI_RED);
		GUI_DispStringHCenterAt("3b Sieger!", xSize / 2, 20);		
		GUI_SetColor(GUI_BLUE);
		GUI_DispStringHCenterAt("Yeah - Geschafft!",   xSize / 2, 50);

		GUI_SetColor(GUI_GREEN);
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "Nach %u Versuchen!", versuche-1);
		GUI_DispStringHCenterAt(buffer,   xSize / 2, 80);

		GUI_SetColor(GUI_BLACK);
		GUI_SetFont(GUI_FONT_20F_ASCII);


		const SBewertung bewertungen[] = {
			{ .limit=1, .text="Fast unmoeglich!", },
			{ .limit=2, .text="Das war viel Glueck!", },
			{ .limit=3, .text="Supi, duper!!", },
			{ .limit=5, .text="Immer noch Glueck!", },
			{ .limit=7, .text="Du bis klug!", },
			{ .limit=8, .text="Fast optimal!", },
			{ .limit=9, .text="Gut!", },
			{ .limit=10, .text="Es geht besser!", },
			{ .limit=12, .text="Ueben!", },
			{ .limit=15, .text="Jetzt aber!", },
			{ .limit=20, .text="Was machst Du?", },
			{ .limit=30, .text="Schlecht!", },
			{ .limit=50, .text="Raetst Du?", },
			{ .limit=100, .text="Jetzt konntest\nDu alle probieren!", },
			{ .limit=0, .text="Ich gebe auf!", },
		};
		SBewertung const* bewertung=&bewertungen[0];
		while (bewertung->limit && bewertung->limit<versuche)
			bewertung++;		
		GUI_DispStringHCenterAt(bewertung->text, xPos, 110);

		GUI_DispStringHCenterAt(
			"Mit maximal 7 Versuchen\n"
			"schafft es ein sehr\n"
			"kluger Spieler\n"
			"immer!!!\n"
			"Noch mal?\n"
			"Taste druecken!\n"
			, xPos, 190);
	
		waitForButton();

	}
}

/*************************** End of file ****************************/
