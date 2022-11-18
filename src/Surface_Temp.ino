/*
 * Project Surface_Temp
 * Description: A code to measure data from the MLX90614 with the Particle Boron and publish that data to the cloud. 
 * Author: Travis Morrison
 * Date: Began on 10-13-2021
 * Last edits: 10-14-2021
 * 
 * This code depends on the MLX90614 library developed by Adafruit and uses a webhook intergration with google sheets. 
 */
//------------------------------------------------------------------------------------------------------------------------
// Libraries

//Include the MLX library
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
//------------------------------------------------------------------------------------------------------------------------
// Define global variables 

float ObjectTempC_tot = 0;
float BodyTempC_tot = 0;
float Max_temp = 50; // max temperature allowed
float ObjectTempC_Avg = 0;
float BodyTempC_Avg = 0;
float ObjectTempC = 0;
float BodyTempC = 0;
float N_obs = 0; // counter for good obs
int Avg_period = 10; // in minutes
int Obs_Freq = 1; //Frequency of measurements made in minutes

time_t t;
String data_timestamp_s;
char * data_time_c;
//------------------------------------------------------------------------------------------------------------------------
// Set Particle Board Properties
SYSTEM_THREAD(ENABLED); //allows the code to run before connecting to the cloud and will run without cloud conncetion
SerialLogHandler logHandler; //Allows google sheets function

// The event name to publish with has to be same as webhook
const char *eventName = "sheetTest1";

//------------------------------------------------------------------------------------------------------------------------
//Decleration of functions~ tells the compiler which functions are available and how to use them
void PublishToGoogleSheet();
//------------------------------------------------------------------------------------------------------------------------
// Setup loop, run 1x
void setup() {
    //Open serial connection for debug
    Serial.begin(9600);

    //Set clock to UTC time~ not is use at the moment
    t = Time.now();
    Time.zone(-0);  // setup a time zone (UTC), which is part of the ISO8601 format
    Time.format(t, TIME_FORMAT_ISO8601_FULL); // 2004-01-10T08:22:04-05:15
    
    //Start MLX 
    mlx.begin();
}

//------------------------------------------------------------------------------------------------------------------------
// Main loop
void loop() {

// loop 1x per min taking 1 value per loop
for (int ii = 1; ii <= Avg_period; ii = ii + Obs_Freq) {
    
    // Read the body temperature in Deg C.
    ObjectTempC = mlx.readObjectTempC();
    // Pausing for a quick second to see if this helps data drop issues
    delay(50ms);
    // Read the surface temperature in Deg C.
    BodyTempC = mlx.readAmbientTempC();
    // filter out bad body temps
    if(ObjectTempC < Max_temp && BodyTempC < Max_temp ) {
        // running total
        ObjectTempC_tot = ObjectTempC_tot+ObjectTempC;
        BodyTempC_tot = BodyTempC_tot+BodyTempC;
        // add 1 to the counter
        N_obs = N_obs + 1;
    }
    //wait 1 min
    delay(60s);
}
// Error catch if no good obs were made
if (N_obs == 0){
    //write -999 for nan values
    ObjectTempC_Avg = -999;
    BodyTempC_Avg = -999;
}else {// good obs were made
    // Compute average over the 10-min window
    ObjectTempC_Avg = ObjectTempC_tot/N_obs;
    BodyTempC_Avg = BodyTempC_tot/N_obs;
}

//Work in Progress ~ writing to data collection vs. time of write - not a huge deal since always connected
/*
//Get the time of data write
data_timestamp_s = Time.timeStr().c_str();
data_timestamp_s.toCharArray(data_time_c, data_timestamp_s.length());
*/

// Calls Publish to Google sheet function
PublishToGoogleSheet();

// Reset running total and counter
ObjectTempC_tot = 0; 
BodyTempC_tot = 0;
N_obs = 0;
}
//------------------------------------------------------------------------------------------------------------------------
// Functions

//Function to Publish to google sheets function
void PublishToGoogleSheet() {
    char buf[128];
    //snprintf(buf, sizeof(buf),"[%s,%.2f,%.2f]",data_time_c, ObjectTempC, BodyTempC);
    snprintf(buf, sizeof(buf),"[%.2f,%.2f]", ObjectTempC_Avg, BodyTempC_Avg);
    Particle.publish(eventName, buf, PRIVATE);
    Log.info("published: %s", buf);
}