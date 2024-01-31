#include <stdio.h>
#include <Adafruit_NeoPixel.h> 
#include <stdlib.h>
#include <time.h> // for random


#define GRID_SIZE 8
#define game_speed 200 //milliseconds
#define upPIN 5
#define leftPIN 6
#define rightPIN 7
#define downPIN 8


#define fps 30
const int data = 2;
const int store = 3;
const int shift = 4;

typedef struct {
    int length;
    int apples;
    int *x_list;
    int *y_list;
} Snake;

typedef struct {
    int x;
    int y;
} Apple;


// Function prototypes
void display();
void translateStatusToRowData();
void init_snake(Snake *snake);
void init_apple(Apple *apple);
char move_snake(Snake *snake, char input);
void update_status(Apple *apple, Snake *snake);
char move_apple(Apple *apple, Snake *snake);
void set_one_led(int x_pos, int y_pos, int R, int G);
void SetOutputPins(unsigned char cs, unsigned char data, unsigned char clk);
void CheckAndChangeDirection();

Snake snake;
Apple apple;
char direction = 'r'; // Initialize direction as right

int KEY_value;
unsigned char status[GRID_SIZE][GRID_SIZE];  // Size can be adjusted as needed
int rowSelect[8] = {127, 191, 223, 239, 247, 251, 253, 254};
int rowData[8];

void setup() {
    pinMode(rightPIN, INPUT);
    pinMode(upPIN, INPUT);
    pinMode(downPIN, INPUT);
    pinMode(leftPIN, INPUT);

    Serial.begin(9600);
    // 74HC595
    pinMode(data, OUTPUT); // data
    pinMode(store, OUTPUT); // store
    pinMode(shift, OUTPUT); // shift


    srand(time(NULL)); // Seed the random number generator\
  
    init_apple(&apple);
    init_snake(&snake);

    update_status(&apple, &snake);
    display();
  
}

void init_snake(Snake *snake) {
    int initial_length = 3;
    snake->length = initial_length;
    snake->apples = 0;
    snake->x_list = (int *)malloc(initial_length * sizeof(int));
    snake->y_list = (int *)malloc(initial_length * sizeof(int));

    if (snake->x_list == NULL || snake->y_list == NULL) {
        // Handle memory allocation failure
        // For example, you might want to exit the program
        exit(EXIT_FAILURE);
    }

    // Initialize the position arrays
    for (int i = 0; i < initial_length; ++i) {
        snake->x_list[i] = 0;
        snake->y_list[i] = i;
    }
    // Esto hizo que las posiciones sean [0, 0], [0, 1], [0, 2]
    // osea vertical, con la cabeza hacia arriba
}

void init_apple(Apple *apple){
    apple->x = GRID_SIZE/2;
    apple->y = GRID_SIZE/2;
}

char move_snake(Snake *snake, char input) {
    int head_x = snake->x_list[0];
    int head_y = snake->y_list[0];

    if (input == 'u') {
        head_y--;
    } else if (input == 'd') {
        head_y++;
    } else if (input == 'l') {
        head_x--;
    } else if (input == 'r') {
        head_x++;
    }
    
    // Shift body, now the first and second links are the same
    for (int i = snake->length - 1; i > 0; i--) {
            snake->x_list[i] = snake->x_list[i - 1];
            snake->y_list[i] = snake->y_list[i - 1];
    }

    // Update snake head, now the first link is the correct head
    snake->x_list[0] = head_x;
    snake->y_list[0] = head_y;

    // Check collision with walls
    if (head_x < 0 || head_x >= GRID_SIZE || head_y < 0 || head_y >= GRID_SIZE) {
        return 'L'; // Game over
    }

    // Check collision with self
    for (int i = 1; i < snake->length; i++) {
        if (head_x == snake->x_list[i] && head_y == snake->y_list[i]) {
            return 'L'; // Game over
        }
    }

    return 'C';
}

void update_status(Apple *apple, Snake *snake) {
     // Clear the grid
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            status[i][j] = 0;
        }
    }

    // Mark snake positions
    for (int i = 0; i < snake->length; i++) {
        //status[rows][columns] so status[y][x]
        status[snake->y_list[i]][snake->x_list[i]] = 1;
    }

    // Mark apple positions
    status[apple->y][apple->x] = 1;
}

char move_apple(Apple *apple, Snake *snake) {
    int possible_positions[GRID_SIZE * GRID_SIZE][2]; // matrix for (x, y)
    int available_positions = 0;

    // no need to erase apple, the snake's head is already in its place

    // Find possible positions
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (status[i][j] == 0) { // status[i][j] = status[y][x]
                possible_positions[available_positions][0] = j; // x
                possible_positions[available_positions][1] = i; // y
                available_positions++;
            }
        }
    }

    // Choose a random position
    if (available_positions > 0) {
        int randIndex = rand() % available_positions;
        apple->x = possible_positions[randIndex][0];
        apple->y = possible_positions[randIndex][1];
        return 'C'; // Continue
    } else {
        return 'W'; // Win
    }
}

void display()
{
  translateStatusToRowData();
  for(int i=0; i<8; i++)
  {
    digitalWrite(store, LOW);
    shiftOut(data, shift, LSBFIRST, rowData[i]);
    shiftOut(data, shift, LSBFIRST, rowSelect[i]);
    digitalWrite(store, HIGH);
  }
}

void translateStatusToRowData() {
    for (int row = 0; row < 8; row++) {
        rowData[row] = 0; // Initialize the current row's data to 0
        for (int col = 0; col < 8; col++) {
            // Shift the bit into the correct position and add it to the current row's data
            rowData[row] |= status[row][col] << (7 - col);
        }
    }
}

void CheckAndChangeDirection() {
    if (digitalRead(upPIN) == HIGH && direction != 'd') {
        direction = 'u';
    } else if (digitalRead(downPIN) == HIGH && direction != 'u') {
        direction = 'd';
    } else if (digitalRead(leftPIN) == HIGH && direction != 'r') {
        direction = 'l';
    } else if (digitalRead(rightPIN) == HIGH && direction != 'l') {
        direction = 'r';
    }
}
  

void loop() {
    CheckAndChangeDirection(); 

    if (move_snake(&snake, direction) == 'L') {
        // Game over: you lost
        while(true) { delay(1000); } // Stop everything, maybe flash some LEDs?
    }

    update_status(&apple, &snake); // Snake has moved

    if (snake.x_list[0] == apple.x && snake.y_list[0] == apple.y) {
        // Snake has eaten an apple
        snake.length++;
        snake.apples++;

        update_status(&apple, &snake); // apple moved

        if (move_apple(&apple, &snake) == 'W') {
            // Game over: you win
            while(true) { delay(1000); } // Stop everything, maybe celebrate?
        }
    }

    for(int i=0; i<fps; i++){
        display(); //  MOSTRAR IMAGEN
        delay(game_speed/fps); // Delay for game speed
    }
}
