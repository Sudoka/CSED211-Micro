#include <stdio.h>

#define  N         7 	/* the array size of buttons */

void print_info(void);	/* print information ftn */
void press_button(int []); /* button pressing ftn */ 
void move_ele(int [], int *); /* elevator movement ftn */
void first_floor(int [], int *);  /* the ftn for elevator is at the first floor */
void second_floor(int [], int *); /* the ftn for elevator is at the second floor */
void third_floor(int [], int *);	/* the ftn for elevator is at the third floor */

/* the main ftn */
int main(void)
{
	int buttons[N] = {0}; /* to save up to seven buttons */
	int i/*for loop*/, flag/*chek for elevator is still or moving */, floor/*the variable for representing the floor*/; 
	char yn; /* for yes or no */
	
	print_info(); /* call print information ftn */
	first_floor(buttons, &floor); /* the initial state is the first floor. */
	
	while(1){
		flag = 0; /* the initial value is that the elevator is still */
		press_button(buttons);		  /* call press button ftn */
		move_ele(buttons, &floor);	/* call elevator movement ftn */
		
		/* check if buttons are pressed */
		for(i = 0 ; i < N ; ++i){
			if(buttons[i] == 1)
				flag = 1;
		}
		
		/* when buttons are pressed */
		if(flag){
			printf("The elevator keeps going...\n");
			continue; /* skip things under this */
		}
		
		/* when buttons are not pressed */
		while(!flag){
			printf("The elevator is still...\n");		
			/* ask if the program will be finished */
			printf("Do you want to quit this(y/n)? ");
			scanf("\n%c", &yn);
			/* when the progam is finished */
			if(yn == 'y' || yn == 'Y'){
				printf("\nGoodbye!\n");
				system("pause");
				exit(0);
			}
			/* when the program is not finished */
			else if(yn =='n' || yn == 'N')
				break;
			/* when the input is wrong */
			else
				printf("the input is wrong!\n");
		}
	}
	
	return 0;
}

/* print information ftn */
void print_info(void)
{
	printf("%s",
		"The elevator program!\n"
		"---------------------------------------------------\n"
		"There are total seven buttons to move the elevator.\n"
		"This is the explanation of buttons.\n"
		"\'0\' is the inside button to move the 1st floor.\n"
		"\'1\' is the inside button to move the 2nd floor.\n"
		"\'2\' is the inside button to move the 3rd floor.\n"
		"\'3\' is the outside button in the 1st floor to move upward.\n"
		"\'4\' is the outside button in the 2nd floor to move upward.\n"
		"\'5\' is the outside button in the 2nd floor to move downward.\n"
		"\'6\' is the outside button in the 3rd floor to move downward.\n\n"
		"if you want to stop pressing, input \'9\'\n" );	
}

/* the ftn to take inputs up to seven(pressing buttons) */
void press_button(int buttons[])
{
	int i, input;
	
	for(i = 0 ; i < N ; ++i){
		
		while(1){
			printf("press a button : ");
			scanf("%d", &input);
			
			/* when the # of input is 9, there will be no more inputs */
			if(input == 9){
				i = N;	/* to escape the 'for-loop' */
				break;	/* to escape the 'while-loop' */
			}
			
			/* when the # of input is over 6 execept 9. */	
			if(input >= N){
				printf("Wrong input!\n");
				continue;
			}
			
			/* when the button is not chosen before */	
			if(buttons[input] == 0){
				buttons[input] = 1;
				break;	/* escape the 'while-loop' */
			}
			
			/* when the button is chosen before, take the input again. */	
			printf("This button is already chosen!\n");
		}
	}
}


void move_ele(int buttons[], int* floor)
{
	/***************************************************/
	/* When the elevator is currently at the 1st floor */
	/***************************************************/
	if(*floor == 1){
		/*************************/
		/* Move to the 2nd floor */
		/*************************/
		
		/* When the '1' button is chosen */
		/* : same the 1st row in the truth table of the digital desgin */
		if(buttons[1] == 1)
			second_floor(buttons, floor);			
		
		/* When the '1' button is not chosen but the '4' button is chosen */
		/* : same the 2nd row in the truth table of the digital desgin */
		else if(buttons[1] == 0 && buttons[4] == 1)
			second_floor(buttons, floor);
		
		/* When the '1' and '2' and '6' are not chosen but the '5' button is chosen */
		/* : same the 3rd row in the truth table of the digital desgin */
		else if(buttons[1] == 0 && buttons[2] == 0 && buttons[5] == 1 && buttons[6] == 0)
			second_floor(buttons, floor);			
		
		/*************************/
		/* Move to the 3rd floor */
		/*************************/
		
		/* When the '1' and '4' are not chosen but the '2' button is chosen */
		/* : same the 4th row in the truth table of the digital desgin */
		else if(buttons[1] == 0 && buttons[2] == 1 && buttons[4] == 0)
			third_floor(buttons, floor);
		
		/* When the '1' and '2' and '4' are not chosen but the '6' button is chosen */
		/* : same the 5th row in the truth table of the digital desgin */
		else if(buttons[1] == 0 && buttons[2] == 0 && buttons[4] == 0 && buttons[6] == 1)
			third_floor(buttons, floor);
		
		/* the case is not in things above */
		else
			first_floor(buttons, floor);
	}
	
	/***************************************************/
	/* When the elevator is currently at the 2nd floor */
	/***************************************************/
	else if(*floor == 2){
		
		/*************************/
		/* Move to the 1st floor */
		/*************************/
		
		/* When the '0' button is chosen */
		/* : same the 6th row in the truth table of the digital desgin */
		if(buttons[0] == 1)
			first_floor(buttons, floor);	
		
		/* When the '0' and '2' are not chosen but the '3' button is chosen */
		/* : same the 7th row in the truth table of the digital desgin */
		else if(buttons[0] == 0 && buttons[2] == 0 && buttons[3] == 1)
			first_floor(buttons, floor);
		
		/*************************/
		/* Move to the 3rd floor */
		/*************************/
		
		/* When the '0' button is not chosen but the '2' button is chosen */
		/* : same the 8th row in the truth table of the digital desgin */
		else if(buttons[0] == 0 && buttons[2] == 1)
			third_floor(buttons, floor);
		
		/* When the '0' and '2' and '3' are not chosen but the '6' button is chosen */
		/* : same the 9th row in the truth table of the digital desgin */
		else if(buttons[0] == 0 && buttons[2] == 0 && buttons[3] == 0 && buttons[6] == 1)
			first_floor(buttons, floor);
		
		/* the case is not in things above */
		else
			second_floor(buttons, floor);
	}
	
	/***************************************************/
	/* When the elevator is currently at the 3rd floor */
	/***************************************************/
	else if(*floor == 3){
		
		/*************************/
		/* Move to the 1st floor */
		/*************************/
		
		/* When the '1' and '5' are not chosen but the '0' button is chosen */
		/* : same the 10th row in the truth table of the digital desgin */
		if(buttons[0] == 1 && buttons[1] == 0 && buttons[5] == 0)
			first_floor(buttons, floor);				
		/* When the '0' and '1' and '5' are not chosen but the '3' button is chosen */
		/* : same the 11th row in the truth table of the digital desgin */
		else if(buttons[0] == 0 && buttons[1] == 0 && buttons[3] == 1 && buttons[5] == 0)
			first_floor(buttons, floor);			
		
		/*************************/
		/* Move to the 2nd floor */
		/*************************/
		
		/* When the '1' button is chosen */
		/* : same the 12th row in the truth table of the digital desgin */
		else if(buttons[1] == 1)
			second_floor(buttons, floor);			
		/* When the '1' button is not chosen but the '5' button is chosen */
		/* : same the 13th row in the truth table of the digital desgin */
		else if(buttons[1] == 0 && buttons[5] == 1)
			second_floor(buttons, floor);			
		/* When the '0' and '1' are not chosen but the '4' button is chosen */
		/* : same the 14th row in the truth table of the digital desgin */
		else if(buttons[0] == 0 && buttons[1] == 0 && buttons[4] == 1)
			second_floor(buttons, floor);
		
		/* the case is not in things above */
		else
			third_floor(buttons, floor);			
	}
	else{
		printf("Unpredicted error!\n");
		printf("Good bye!\n");
		system("pause");
		exit(0);
	}
}

/* the ftn for when the elevator is at the first floor */
void first_floor(int buttons[], int *floor)
{
	printf("Here is the first floor.\n");
	*floor = 1;
	/* reset buttons to go to the 1st floor */
	buttons[0] = 0;
	buttons[3] = 0;
}

/* the ftn for when the elevator is at the second floor */
void second_floor(int buttons[], int *floor)
{
	printf("Here is the second floor.\n");
	*floor = 2;
	/* reset buttons to go to the 2nd floor */
	buttons[1] = 0;
	buttons[4] = 0;
	buttons[5] = 0;
}

/* the ftn for when the elevator is at the third floor */
void third_floor(int buttons[], int *floor)
{
	printf("Here is the third floor.\n");
	*floor = 3;
	/* reset buttons to go to the 3rd floor */
	buttons[2] = 0;
	buttons[6] = 0;
}