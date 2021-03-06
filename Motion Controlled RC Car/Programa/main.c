#include <stm32f10x.h>//definición de los nombres

int n;
int out;
int led;

//THERE SHOULD BE A DELAY BETWEEN TIMER ON AND START OF CONVERSION
//resolution (12 bits) (Vref + aprox 3 Volts) (Vref- = 0 V)

int main(void)
{
	led = 0;
	n = 0;
	out = 0;


	RCC->APB2ENR = 0x214;//Reset and clock control: Del registro RCC se accede a la parte APB2ENR. activa adc, puerto C y A
	GPIOA->CRL = 0; //PUERTO A, que tiene el ADC, configurado como INPUT, ANALOGO
	GPIOC->CRL = 0x22222222;//PUERTO C LOW, config en general purpose output, max speed 2MHz
	GPIOC->CRH = 0x22222222;//PUERTO C HIGH, config en general purpose output, max speed 2MHz

	//Configuración del timer
    RCC->APB1ENR |= 0x1;		//Habilita el reloj del timer 2.
    //TIM2->PSC = 23999;			// divisor de la frecuencia de reloj dle timer
    TIM2->PSC=100;			// divisor de la frecuencia de reloj dle timer
    TIM2->ARR = 2;
	TIM2->CR1  |= 0x1;			//habilita el timer y lo arranca!.


	NVIC_EnableIRQ(ADC1_IRQn);

	ADC1->CR2 = 0x1; //ADC ON , single mode(cont=0),right alignment (bit 11)
	ADC1->CR1 = (1<<5); //habilita EOC, que nos permite saber que acabo la conversion
	ADC1->SMPR2 = 0x000; //sample time for each channel set to 1.5 cycles
	ADC1->SQR1 = 0;	// total number of conversions (one conversion is a 0 in the bits #20-23), other bits are order of the conversion of channel, from last to first going down
	ADC1->SQR2 = 0; // no channels converted in this range (from 12th to 7th)

	GPIOC->ODR = 0x0; //valor inicial

    while(1) // repite todo esto
    {

    	ADC1->SQR3 = 0x1; // in the first 5 bits you put the channel # of the channel you want to be converted first, in this case channel 2 (second in position)
        ADC1->CR2 |= ADC_CR2_ADON; //comenzar la conversion


		while (n<30) // crea un retardo para que le de tiempo a convertir
		{
		n  = n + 1;
		}
		n = 0;

    			if (ADC1->DR > 2100)	// limite superior de la conversion que controla el movimiento frontal/trasero
    			{
    				out = 0x1;
    			}
    			else if ((ADC1->DR < 2100) & (ADC1->DR > 2060)) //rango intermedio para el avance
    			{
        			if(TIM2->SR & TIM_SR_UIF )	//Verifica el flag de actualización del timer 2
        			{
        				TIM2->SR &= ~(1<<0); // Resetea el flag UIF
        				led ^= 0x1; //led on y para alante con velocidad intermedia
        				out = led;
        			}
    			}
    			else if ((ADC1->DR > 1950) & (ADC1->DR < 1990))
    			{
        			if(TIM2->SR & TIM_SR_UIF )	//Verifica el flag de actualización del timer 2
        			{
        				TIM2->SR &= ~(1<<0); // Resetea el flag UIF
        				led ^= 0x2; // led on y para atras con velocidad intermedia
        				out = led;
        			}
    			}
    			else if (ADC1->DR < 1950) // para atras con velocidad maxima
    			{
    				out = 0x2;
    			}
    			else
    			{
    				out = 0x0; // posicion inmovil
    			}


    	ADC1->SQR3 = 0x2; // in the first 5 bits you put the channel # of the channel you want to be converted first, in this case channel 2 (second in position)
    	ADC1->CR2 |= ADC_CR2_ADON; //start conversion del otro eje



    			while (n<30 ) // crea un retardo para que le de tiempo convertir
    			{
    			n  = n + 1;
    			}
    			    n = 0;



    			if (ADC1->DR > 2080) //se mueve a la derecha si se da esa condicion
    			{
    				out |= 0x4;//hace una OR con todo el dato del eje anterior para que no afecte el dato del otro eje
    			}
    			else if (ADC1->DR < 1990) // se mueve a la izquierda si se da esta condicion
    			{
    				out |= 0x8;
    			}
    			else
    			{
    				out |= 0x0; // no dobla
    			}

    		    GPIOC->ODR = out;// pone el dato resultante en la salida del puerto C
    }
}

void ADC1_IRQHandler(void)

{
		ADC1->SR = 0; // si se da la interrrupcion, resetea el pin de estatus
}
