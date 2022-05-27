#include <AccelStepper.h> // Load the AccelStepper library
#include <ArduinoBLE.h>

#define motorPin1 8  // IN1 pin on the ULN2003A driver
#define motorPin2 9  // IN2 pin on the ULN2003A driver
#define motorPin3 10 // IN3 pin on the ULN2003A driver
#define motorPin4 11 // IN4 pin on the ULN2003A driver

// command type
#define START 1
#define STOP 2
#define REWIND 3
#define BACKWARD 4
#define FORWARD 5

// AccelStepper stepper(AccelStepper::HALF4WIRE, motorPin4, motorPin2, motorPin3, motorPin1);
AccelStepper stepper(AccelStepper::HALF4WIRE, motorPin1, motorPin3, motorPin2, motorPin4);

BLEService realisStartrackerBluetoothService("4587B400-28DF-4DA5-B617-BC2B58CE7930");
BLEUnsignedIntCharacteristic commandCharacteristic("4587B401-28DF-4DA5-B617-BC2B58CE7930", BLERead | BLEWrite);
BLEStringCharacteristic stateCharacteristic("4587B402-28DF-4DA5-B617-BC2B58CE7930", BLERead | BLENotify, 512);
BLEDoubleCharacteristic trackingSpeedCharacteristic("4587B403-28DF-4DA5-B617-BC2B58CE7930", BLERead | BLEWrite);

const double MAX_TRACKING_SPEED = 1000.0;
const double TRACKING_SPEED = 271.71;
const double MOVING_SPEED = 900.0;

long positionCheckInterval = 5000L;
long previousTime = millis();

void writeStateToBLE(String message)
{
    stateCharacteristic.writeValue(message);
}

void writeStatePositionToBLE()
{
    long currentTime = millis();
    if ((currentTime - previousTime) > positionCheckInterval)
    {
        previousTime = currentTime;
        writeStateToBLE(String(stepper.currentPosition()));
    }
}

void setup()
{
    Serial.begin(115200);
    // while (!Serial)
    //     ;

    if (!BLE.begin())
    {
        Serial.println("isStarting Bluetooth® Low Energy module failed!");
        while (1)
            ;
    }

    realisStartrackerBluetoothService.addCharacteristic(commandCharacteristic);
    realisStartrackerBluetoothService.addCharacteristic(stateCharacteristic);
    realisStartrackerBluetoothService.addCharacteristic(trackingSpeedCharacteristic);

    BLE.setLocalName("RealisStartrackerBluetoothController");
    BLE.setAdvertisedService(realisStartrackerBluetoothService);
    BLE.addService(realisStartrackerBluetoothService);
    BLE.advertise();

    writeStateToBLE("Ready!!");
    writeStateToBLE("CMD:SPEED:" + String(TRACKING_SPEED));

    Serial.println(("Bluetooth® device active, waiting for connections..."));

    // setup for stepper motor
    stepper.setMaxSpeed(MAX_TRACKING_SPEED);
    // stepper.setAcceleration(100);
    // stepper.setSpeed(speed);
}

unsigned long at = millis();
double trackingSpeed = TRACKING_SPEED;

boolean isStart = false;
boolean isRewind = false;
boolean isBackward = false;

void loop()
{
    BLE.poll();

    int cmd = 0;
    if (commandCharacteristic.written())
    {
        cmd = commandCharacteristic.value();
        Serial.println(commandCharacteristic.value());
    }

    if (trackingSpeedCharacteristic.written())
    {
        trackingSpeed = trackingSpeedCharacteristic.value();
        Serial.print("recv : tracking speed = ");
        Serial.println(trackingSpeed);
    }

    switch (cmd)
    {
    case START:
        isStart = true;
        isRewind = false;
        isBackward = false;
        Serial.print("START .. speed=");
        Serial.println(trackingSpeed);
        writeStateToBLE("CMD:SPEED:" + String(trackingSpeed));
        break;

    case STOP:
        isStart = false;
        isRewind = false;
        isBackward = false;
        stepper.stop();
        Serial.print("STOP");

        // backward로 시작점 보다 이전이라면 새로운 시작점으로 셋팅
        if (stepper.currentPosition() < 0)
        {
            stepper.setCurrentPosition(0);
        }
        break;

    case REWIND:
        isStart = false;
        isRewind = true;
        isBackward = false;
        Serial.print("REWIND .. speed=");
        Serial.println(MOVING_SPEED);
        break;

    case BACKWARD:
        isStart = false;
        isRewind = false;
        isBackward = true;
        Serial.println("BACKWARD .. speed=");
        Serial.println(MOVING_SPEED);
        break;
    default:
        break;
    }

    if (isStart)
    {
        stepper.setSpeed(trackingSpeed);
        stepper.runSpeed();
        writeStatePositionToBLE();
    }
    else if (isRewind)
    {

        stepper.moveTo(-stepper.currentPosition());
        stepper.setSpeed(MOVING_SPEED);
        stepper.runSpeedToPosition();

        writeStatePositionToBLE();
        if (stepper.currentPosition() == 0L)
        {
            writeStateToBLE("CMD:COMPLETED_REWIND");
            isRewind = false;
        }
    }
    else if (isBackward)
    {
        stepper.setSpeed(-MOVING_SPEED);
        stepper.runSpeed();

        writeStatePositionToBLE();
    }
}