#include <AccelStepper.h>				// Stepper Control Library
#include <SerialCommand.h>				// Serial Commands Library
#include <Wire.h>					// Wire for LM75
#include <LM75.h>					// LM75 Library
#include <OneWire.h>			                //
#include <DallasTemperature.h>                          // 1W For DS18B20 Temp sensor

#define M0		8				// Control Driver resolution
#define M1		7				// Control Driver resolution
#define SLEEP 	4   					// Control Driver ENABLE \ DISABLE
#define ONE_WIRE_BUS	11
#define DEVICE_RESPONSE "E24.Focuser"	// equal to DEVICE_RESPONSE

//////////////////////////////////////////////////////////////////

unsigned long mov 			= 0;	// Current Position
unsigned long set_current_pos;			// Desired Position

bool is_mov				= false;	// IsMoving
bool sleep_state			= false;	// Sleep 
bool TempComp 				= false;	// TempComp disabled by default
bool Revers                             = false;	// Reverse direction

double temperature1;                    // Sensor #1 Temp
double temperature2;                    // Sensor #2 Temp
double temperature3;                    // Sensor #3 Temp
double _temperature;			// Control Temp for TempComp

unsigned int  spd	      = 800;	// Default Speed pulse per second
unsigned int  accel 	      = 400;	// Default Acceleration
unsigned int  sleep_delay     = 0;	// Sleep delay in sec 0 disabled
unsigned long last_move_time  = 0;	// Time for auto sleep

byte res 		      = 4;	// Default MicroStep

//////////////////////////////////////////////////////////////////

AccelStepper stepper(1, A2, A3);		// pin A2 = step, pin A3 = direction
SerialCommand sCmd;				// The SerialCommand object
LM75 sensor;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
	Serial.begin(9600);	
        sensors.begin();                    	// Turn on temp sensor
	Wire.begin();
	LM75 sensor;

	pinMode(SLEEP, OUTPUT); 
	digitalWrite(SLEEP, HIGH);

	setresolution();                   	// Set SM resolution
	stepper.setMaxSpeed(spd);		// Set SM Speed
	stepper.setAcceleration(accel);		// Set SM Acceleration

	sCmd.addCommand("#",  HelloHS);		// Check DEVICE_RESPONSE

	sCmd.addCommand("S",  setSpeed);	// Speed management
	sCmd.addCommand("A",  setAccel);	// Acceleration management
	sCmd.addCommand("B",  setRes);		// Set SM resolution
	sCmd.addCommand("R",  setPos);		// Set SM position
	sCmd.addCommand("C",  setSLPTime);	// Set sleep delay in sec
	sCmd.addCommand("Z",  setRevers);	// Revers

	sCmd.addCommand("M",  moveSM);		// Move command
	sCmd.addCommand("T1", getTemp1);        // Read temp #1 sensor
	sCmd.addCommand("T2", getTemp2);	// Read temp #1 sensor
	sCmd.addCommand("T3", getTemp3);	// Read temp #1 sensor
	sCmd.addCommand("H",  Halt);	        // Halt SM
	sCmd.addCommand("I",  getIsMov);	// Is SM moving true \ false
	sCmd.addCommand("P",  getPos);	        // Current Position

	sCmd.setDefaultHandler(unrecog);	// All other: send ERR

}

void loop() 
{
	if(is_mov == false)
	{
		if(millis() % 5000 == 0)       // Re-read sensors every 5 sec
		{
                        sensors.requestTemperatures();
                        
			temperature1 = sensor.temp();
			temperature2 = sensors.getTempCByIndex(0); // Modify to use with 2nd sensor
			
                        temperature3 = sensor.temp(); // Modify to use with 3rd sensor
		}
		
		if(sleep_delay > 0) // Checks: If sleep delay is on
		{                    
			check_time();			
		}
	}

	sCmd.readSerial();			// 1st: read input from com port
	stepper_run();
}

// # Handshake
void HelloHS()
{
	Serial.println(F(DEVICE_RESPONSE));
}
// S Set Speed
void setSpeed()
{
	char *arg = sCmd.next();
	spd = atoi (arg);
	stepper.setMaxSpeed(spd);
	Serial.println("S");
}
// A Set Acceleration
void setAccel()
{
	char *arg = sCmd.next();
	accel = atoi (arg);
	stepper.setAcceleration(accel);
	Serial.println("A");
}

//B Set Resolution
void setRes()
{
	char *arg = sCmd.next();
	res = atol(arg);
	setresolution();
	Serial.println("B");
}

// R Set Position
void setPos()
{
  char *arg = sCmd.next();
  set_current_pos = atol(arg);
  stepper.setCurrentPosition(set_current_pos);
  stepper.moveTo(set_current_pos);
  Serial.println("R");
}

// C Set Sleep Time
void setSLPTime()
{
	char *arg = sCmd.next();
	sleep_delay = atol(arg);
	Serial.println("C");
}

// Command: Z
void setRevers()
{
	char *arg = sCmd.next();
	Revers = atoi(arg);
	stepper.setPinsInverted(Revers);
	Serial.println("Z");
}

// M Command MOVE
void moveSM()
{
	char *arg = sCmd.next();
	last_move_time = millis();
	mov = atol(arg);
	stepper.moveTo(mov);
	Serial.println("M");
}

// T1 Get Temperature 1
void getTemp1()
{
	Serial.print("T:");
	Serial.println(temperature1);
}

// T1 Get Temperature 2
void getTemp2()
{
	Serial.print("T:");
	Serial.println(temperature2);
}

// T1 Get Temperature 3
void getTemp3()
{
	Serial.print("T:");
	Serial.println(temperature3);	
}

// H Halt SM
void Halt()
{
	stepper.moveTo(stepper.currentPosition());
	Serial.println('H');
}

// I IsMoving
void getIsMov()
{
	if (is_mov == 1)
	{
		Serial.println("I:true");
	} 
	else 
	{
		Serial.println("I:false");
	}
}

// P Position
void getPos()
{
	Serial.print("P:");
	Serial.println(stepper.currentPosition());
}

// All other Command: ERROR
void unrecog(const char *command)
{
	Serial.print("ERROR: ");
	Serial.println(command);
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

void stepper_run()
{
	if (stepper.distanceToGo() != 0)
	{
		last_move_time = millis();
		is_mov = true;
		stepper.run();
	}
	else
	{
		is_mov = false;
	}
}

void check_time()
{
	if ((millis() - last_move_time) > sleep_delay*1000 && sleep_state == false)
	{
		digitalWrite(SLEEP, LOW);
		sleep_state = true;
	}
	else if (millis() - last_move_time < sleep_delay*1000 && sleep_state == true)
	{
		digitalWrite(SLEEP, HIGH);
		sleep_state = false;
	}
}
