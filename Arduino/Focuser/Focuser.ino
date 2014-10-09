#include <AccelStepper.h>		// Библиотека управления ШД
#include <SerialCommand.h>		// Читаем ком порт как строки с параметрами
#include <OneWire.h>			//
#include <DallasTemperature.h>  // 1W For DS18B20 Temp sensor

#define ONE_WIRE_BUS	11
#define M0				8		// Control Driver resolution
#define M1				7		// Control Driver resolution
#define SLEEP			4		// Control Driver ENABLE \ DISABLE


#define DEVICE_RESPONSE "Arduino.Open.Focuser"	// must be equal to DEVICE_RESPONSE in Driver.vb

long e_pos;
long mov 					= 0;		// Current Position
long set_current_pos;

bool is_mov					= false;	// IsMoving False or True
bool sleep_state			= false;

double temp;							// Temperature

unsigned int  spd			= 2000;		// Default Speed
unsigned int  accel 		= 500;		// Default Acceleration
unsigned int  sleep_delay  	= 60;			// Set sleep delay in sec
unsigned long last_move_time;

byte res 					= 1;		// 1,2,4,8,16,32
byte res_prev				= 1;		// 1,2,4,8,16,32




AccelStepper stepper(1, A2, A3);		// pin A2 = step, pin A3 = direction
SerialCommand sCmd;						// The SerialCommand object

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup()
{
	pinMode(SLEEP, OUTPUT);
	Serial.begin(9600);						// Com порт на 9600  
	sensors.begin();                    	// Turn on temp sensors
	digitalWrite(SLEEP, HIGH);				// Enable SM
	setresolution();                    	// Set SM resolution
	stepper.setMaxSpeed(spd);				// Set SM Speed (ips)
	stepper.setAcceleration(accel);			// Set SM Acceleration

	sCmd.addCommand("#", HelloHS);			// not_used: Hello and check DEVICE_RESPONSE
	sCmd.addCommand("S", SetSpeed);			// Speed management
	sCmd.addCommand("A", SetAccel);			// Acceleration management
	sCmd.addCommand("M", moveSM);			// Move command
	sCmd.addCommand("T", getTemp);	    	// Read temp
	sCmd.addCommand("H", setHalt);	    	// Halp SM
	sCmd.addCommand("I", getIsMov);			// Is SM moving true \ false
	sCmd.addCommand("P", getPos);	    	// Current Position
	sCmd.addCommand("E", enblSM);			// not_used: Re-enable SM manualy 
	sCmd.addCommand("D", dsblSM);			// not_used: Disable SM manualy
	sCmd.addCommand("B", setRes);			// Set SM resolution
	sCmd.addCommand("R", setPos);			// Set SM
	sCmd.addCommand("Q", getPosE);			// Read SM prev position from EEPROM
	sCmd.addCommand("C", setDisTime);		// Set sleep delay in sec
	sCmd.addCommand("HELP", sendHelp);		// Set sleep delay in sec
	sCmd.setDefaultHandler(unrecognized);	// All other: send ERR
	Serial.println("# Ready #");			// ... and we are ready
}

void loop()
{
	sCmd.readSerial();						// 1st: read input from com port
	stepper_run();							// 3rd: let's go
	if(sleep_delay > 0){                    // If sleep delay is on, waitng for last_move_time - current time > sleep_delay
		check_time();			
	}

	if(millis() % 5000 == 0 && is_mov == false){    // Re-read sensors every 5 sec
		sensors.requestTemperatures();
		temp = sensors.getTempCByIndex(0);
	}

	if(res_prev != res){
		setresolution();
		res_prev = res;
	}
}

void setresolution(){
	switch (res) {
		case 1:{
			pinMode(M0, OUTPUT); digitalWrite(M0, LOW);
			pinMode(M1, OUTPUT); digitalWrite(M1, LOW);
			break;
		}
		case 2:{
			pinMode(M0, OUTPUT); digitalWrite(M0, HIGH);
			pinMode(M1, OUTPUT); digitalWrite(M1, LOW);
			break;
		}
		case 4:{
			pinMode(M0, INPUT);  digitalWrite(M0, LOW);
			pinMode(M1, OUTPUT); digitalWrite(M1, LOW);
			break;
		}
		case 8:{
			pinMode(M0, OUTPUT); digitalWrite(M0, LOW);
			pinMode(M1, OUTPUT); digitalWrite(M1, HIGH);
			break;
		}
		case 16:{
			pinMode(M0, OUTPUT); digitalWrite(M0, HIGH);
			pinMode(M1, OUTPUT); digitalWrite(M1, HIGH);
			break;
		}
		case 32:{
			pinMode(M0, INPUT);  digitalWrite(M0, LOW);
			pinMode(M1, OUTPUT); digitalWrite(M1, HIGH);
			break;
		}
	}
}
///////////////////// Serial Commands ////////////////////////////////////////////

// Command: #
void HelloHS(){
	Serial.print(DEVICE_RESPONSE);
	Serial.print('\n');
}

// Command: S
void SetSpeed(){
	char *arg;
	arg = sCmd.next();
	if (arg != NULL){
		spd = atoi (arg);
		stepper.setMaxSpeed(spd);
		Serial.print("S");
		Serial.print('\n');
	} else {
		Serial.println("Value needed");
	}
}

// Command: A
void SetAccel(){
	char *arg;
	arg = sCmd.next();
	if (arg != NULL){
		accel = atoi (arg);
		stepper.setAcceleration(accel);
		Serial.print("A");
		Serial.print('\n');
	} else {
		Serial.println("Value needed");
	}
}

// Command: M
void moveSM(){
	char *arg;
	arg = sCmd.next();
	last_move_time = millis();
	if (arg != NULL) {
		mov = atol(arg);
		stepper.moveTo(mov);			// 2nd: set target position for SM
		Serial.print("M");
		Serial.print('\n');
	} else {
		Serial.println("No arguments");
	}
}

// Command: T
void getTemp(){
	if (temp == -127){
		Serial.print("T:false");
		Serial.print('\n');
	} else {
		Serial.print("T:");
		Serial.print(temp);
		Serial.print('\n');
	}
}

// Command: H
void setHalt(){
	stepper.moveTo(stepper.currentPosition());
	Serial.print('H');
	Serial.print('\n');
}

// Command: I
void getIsMov(){
	if (is_mov == 1){
		Serial.print("I:true");
		Serial.print('\n');
	} else {
		Serial.print("I:false");
		Serial.print('\n');
	}
}

// Command: P
void getPos(){
	Serial.print("P:");
	Serial.print(stepper.currentPosition());
	Serial.print('\n');  
}

void enblSM(){
	digitalWrite(SLEEP, HIGH);
	Serial.print("ENABLED");
	Serial.print('\n');
}

void dsblSM(){
	digitalWrite(SLEEP, LOW);
	Serial.print("DISABLED");
	Serial.print('\n');
}

// Command: B
void setRes(){
	char *arg;
	arg = sCmd.next();
	if (arg != NULL) {
		res = atol(arg);
		Serial.print("B");
		Serial.print('\n');
	} else {
		Serial.println("No arguments");
	}
}

// Command: R
void setPos(){
	char *arg;
	arg = sCmd.next();
	if (arg != NULL) {
		set_current_pos = atol(arg);
		stepper.setCurrentPosition(set_current_pos);
		stepper.moveTo(set_current_pos);
		Serial.print("R");
		Serial.print('\n');
	} else {
		Serial.println("No arguments");
	}
}

// Command: Q
void getPosE(){
	stepper.setCurrentPosition(e_pos); stepper.moveTo(e_pos);
	Serial.print("Q");
	Serial.print('\n');
}

// Command: C
void setDisTime(){
	char *arg;
	arg = sCmd.next();
	if (arg != NULL) {
		sleep_delay = atol(arg);
		Serial.print("C");
		Serial.print('\n');
	} else {
		Serial.println("No arguments");
	}
}

// Command: other ...
void unrecognized(const char *command){
	Serial.print("ERR: ");
	Serial.print(command);
	Serial.print('\n');
}

///////////////////// End Serial Commands ////////////////////////////////////////

void stepper_run(){
	if (stepper.distanceToGo() != 0){
		last_move_time = millis();
		is_mov = true;
		stepper.run();
	} else if (stepper.distanceToGo() == 0){
		is_mov = false;
	}
}

void check_time(){
	if ((millis() - last_move_time) > sleep_delay*1000 && sleep_state == false){
		digitalWrite(SLEEP, LOW);
		sleep_state = true;
	} else if (millis() - last_move_time < sleep_delay*1000 && sleep_state == true){
		digitalWrite(SLEEP, HIGH);
	sleep_state = false;
	}
}
void sendHelp(){
	Serial.println('# - Hello and check DEVICE_RESPONSE');
	Serial.println('S + Value - Speed management');
	Serial.println('A + Value - Acceleration management');
	Serial.println('M + Value - Move command');
	Serial.println('T - Read temp');
	Serial.println('H - Halp SM');
	Serial.println('I - Is SM moving true or false');
	Serial.println('P - Current Position');
	Serial.println('E - not_used: Re-enable SM manualy ');
	Serial.println('D - not_used: Disable SM manualy');
	Serial.println('B + Value - Set SM resolution');
	Serial.println('R + Value - Set SM position');
	Serial.println('Q - Read SM prev position from EEPROM');
	Serial.println('C - Set sleep delay in sec');
}