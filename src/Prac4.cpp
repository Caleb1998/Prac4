\/*
 * Prac4.cpp
 * 
 * Originall written by Stefan Schröder and Dillion Heald
 * 
 * Adapted for EEE3096S 2019 by Keegan Crankshaw
 * 
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "Prac4.h"

using namespace std;

bool playing = true; // should be set false when paused
bool stopped = false; // If set to true, program should close
unsigned char buffer[2][BUFFER_SIZE][2];
int buffer_location = 0;
bool bufferReading = 0; //using this to switch between column 0 and 1 - the first column
bool threadReady = false; //using this to finish writing the first column at the start of the song, before the column is played


// Configure your interrupts here.



// Don't forget to use debouncing.
void play_pause_isr(void){
	printf("Btn pause/play toggle pressed");
	if(playing==true){
	//pause
	playing=false;
	}
	else{
	//play
	playing=true;
	}
    //Write your logic here
}

void stop_isr(void){
    // Write your logic here
	printf("Btn stop pressed");
	stopped=true;
}

/*
 * Setup Function. Called once 
 */
int setup_gpio(void){
    //Set up wiring Pi
    wiringPiSetup();
    //setting up the buttons
	pinMode(PLAY_BUTTON, INPUT);
	pinMode(STOP_BUTTON,INPUT);   //sets button pins to inputs

	pullUpDnControl(PLAY_BUTTON, PUD_UP);
	pullUpDnControl(STOP_BUTTON, PUD_UP);  //enables pull up resistors on pins -> btn pressed = input goes low

    //setting up the SPI interface
	int wiringPiSPISetup (SPI_CHAN, SPI_SPEED) ;//channel 10 (CE01) and speed=25.6kHz -> eff. speed = 16khz (desired frequency)

   return 0;
}

/* 
 * Thread that handles writing to SPI
 * 
 * You must pause writing to SPI if not playing is true (the player is paused)
 * When calling the function to write to SPI, take note of the last argument.
 * You don't need to use the returned value from the wiring pi SPI function
 * You need to use the buffer_location variable to check when you need to switch buffers
 */
void *playThread(void *threadargs){
    // If the thread isn't ready, don't do anything
    while(!threadReady)
        continue;
    
    //You need to only be playing if the stopped flag is false
    while(!stopped){
        //Code to suspend playing if paused
		//TODOne
        while(paused){continue;}//does nada
	while(!paused){

        //Write the buffer out to SPI
        //TODO
	
        //Do some maths to check if you need to toggle buffers
        buffer_location++;
        	if(buffer_location >= BUFFER_SIZE) {
           	buffer_location = 0;
           	bufferReading = !bufferReading; // switches column one if it finishes one column
        	}
	
	}
    }
    
    pthread_exit(NULL);
}

int main(){
    // Call the setup GPIO function
	if(setup_gpio()==-1){
        return 0;
    }
    
    /* Initialize thread with parameters
     * Set the play thread to have a 99 priority
     * Read https://docs.oracle.com/cd/E19455-01/806-5257/attrib-16/index.html
     */ 
    
    //Write your logic here
	pthread_attr_t tattr;
    pthread_t thread_id;
    int newprio = 99;
    sched_param param;
    
    pthread_attr_init (&tattr);
    pthread_attr_getschedparam (&tattr, &param); /* safe to get existing scheduling param */
    param.sched_priority = newprio; /* set the priority; others are unchanged */
    pthread_attr_setschedparam (&tattr, &param); /* setting the new scheduling param */
    pthread_create(&thread_id, &tattr, playThread, (void *)1); /* with new priority specified *
    
    /*
     * Read from the file, character by character
     * You need to perform two operations for each character read from the file
     * You will require bit shifting
     * 
     * buffer[bufferWriting][counter][0] needs to be set with the control bits
     * as well as the first few bits of audio
     * 
     * buffer[bufferWriting][counter][1] needs to be set with the last audio bits
     * 
     * Don't forget to check if you have pause set or not when writing to the buffer
     * 
     */
     
    // Open the file
    char ch;
    FILE *filePointer;
    printf("%s\n", FILENAME);
    filePointer = fopen(FILENAME, "r"); // read mode

    if (filePointer == NULL) {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }

    int counter = 0;
    int bufferWriting = 0;

    // Have a loop to read from the file
	 while((ch = fgetc(filePointer)) != EOF){
        while(threadReady && bufferWriting==bufferReading && counter==0){
            //waits in here after it has written to a side, and the thread is still reading from the other side
            continue;
        }
        //Set config bits for first 8 bit packet and OR with upper bits
        buffer[bufferWriting][counter][0] =0b01110000|((ch>>6)&0b0011); //D9=0 and D8=0 (input only 8bits) D7 and D6 vary, ch>>6 ensures we get D6&7, D6=LSB
        //Set next 8 bit packet (bits D0-5)
        buffer[bufferWriting][counter][1] = 0b0011111100&(ch<<2); //ch<<2 shifts ch to the correct postion for D0-D5, bitwise & ensures <=8bits and last 2 LSBs are 00(sligthly redundant)

      counter++;
        if(counter >= BUFFER_SIZE+1){
            if(!threadReady){
                threadReady = true;
            }
x
            counter = 0;
            bufferWriting = (bufferWriting+1)%2;
        }

    }
     
    // Close the file
    fclose(filePointer);
    printf("Complete reading"); 
	 
    //Join and exit the playthread
	pthread_join(thread_id, NULL); 
    pthread_exit(NULL);
	
    return 0;
}

