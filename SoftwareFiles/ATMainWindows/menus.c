/*
 * menus.c
 *
 *  Created on: Feb 3, 2016
 *      Author: mitchell
 */


#include "menus.h"
#include "touch.h"
#include "graphics.h"
#include "Fill.h"
#include <string.h>
#include "gps.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "paths.h"
#include <math.h>
#include "past_trips.h"
#include "new_trip.h"
#include "sdcard.h"
#include "Altera_UP_SD_Card_Avalon_Interface.h"

int redraw = 1;
int logNum = 0;
int initial = 1;
int logExists = 0;

void DrawMainMenu()
{
	FilledRectangle(0, 0, 800, 480, DARK_GREEN);
	DrawButton(0, 0, 800, 40, "Adventure Tracker", sizeof ("Adventure Tracker"), WHITE, BLACK);
	DrawButton(50, 50, 375, 225, "Past Trips", sizeof("Past Trips")-1, BLACK, SADDLE_BROWN);
	DrawButton(50, 255, 375, 430, "Easter Eggs", sizeof("Easter Eggs")-1, BLACK, SADDLE_BROWN);
	DrawButton(400, 50, 750, 430, "New Trip", sizeof("New Trip")-1, BLACK, SADDLE_BROWN);
}
////////////////////////////////////////////////////////////////////////////////////////
//CHANGING PAGES
////////////////////////////////////////////////////////////////////////////////////////
void GetNextMenu(Point p)
{
	switch (current_menu_val)
	{
		case (MAINMENU):
			if((p.x > NEWTRIP_X1) && (p.x < NEWTRIP_X2) &&
			   (p.y > NEWTRIP_Y1) && (p.y < NEWTRIP_Y2))
			{
				current_menu_func = &NewTrip;
				redraw = 1;
			}
			else if((p.x > 50) && (p.x < 375) &&
					   (p.y > 255) && (p.y < 430))
			{
				current_menu_func = &EasterEggs;
				redraw = 1;
			}
			else if ((p.x > 50) && (p.x < 375) &&
						(p.y > 50) && (p.y < 225))
			{
				current_menu_func = &PastTrips;
				redraw = 1;
			}
			break;

		case (NEWTRIP):
			if((p.x > NEWTRIP_BACK_X1) && (p.x < NEWTRIP_BACK_X2) &&
			   (p.y > NEWTRIP_BACK_Y1) && (p.y < NEWTRIP_BACK_Y2))
			{
				current_menu_func = &MainMenu;
				redraw = 1;
			}
			break;
		case (EASTEREGGS):
			if((p.x > 0) && (p.x < 800) &&
			   (p.y > 0) && (p.y < 480))
			{
				current_menu_func = &MainMenu;
				redraw = 1;
			}
			break;

		case (PASTTRIPS):
			if((p.x > PAST_TRIP_BACK_X1) && (p.x < PAST_TRIP_BACK_X2) &&
					(p.y > PAST_TRIP_BACK_Y1) && (p.y < PAST_TRIP_BACK_Y2))
			{
				current_menu_func = &MainMenu;
				redraw = 1;
			}
			break;

		default:
			break;
	}
}

//this is for switching logs in past trips
void PrevNext(Point p, int minLog, int maxLogs)
{
		initial = 0;
		//prev
		if((p.x > PREV_X1) && (p.x < PREV_X2) &&
				(p.y > PREV_Y1) && (p.y < PREV_Y2))
		{
			current_menu_func = &PastTrips;
			printf("log = %d, minlog = %d\n", logNum, minLog);
			if (logNum <= 0) {
				logNum = maxLogs-1;
			}
			else
				logNum--;
			redraw = 1;
		}

		//next
		else if((p.x > NEXT_X1) && (p.x < NEXT_X2) &&
			(p.y > NEXT_Y1) && (p.y < NEXT_Y2))
		{
			current_menu_func = &PastTrips;
			printf("log = %d, maxlog = %d\n", logNum, maxLogs);
			if (logNum >= maxLogs - 1) {
				logNum = 0;
			}
			else
				logNum++;
			redraw = 1;
		}
		else
			return;

}

////////////////////////////////////////////////////////////////////////////////////////
//PAGES
////////////////////////////////////////////////////////////////////////////////////////
void MainMenu()
{
	current_menu_val = MAINMENU;
	if(redraw)
	{
		DrawMainMenu();
		redraw = 0;
	}
	int i;
	for(i=0; i<1000; i++);
	Point p = GetRelease();
	//this is here to make sure the SD card is inserted, otherwise program will crash
	Init_SDCard();
	GetNextMenu(p);
	current_menu_func();
}


void EasterEggs()
{
	current_menu_val = EASTEREGGS;
	int i;
	for(i=0; i<10; i++)
	{
		TriangleTest();
	}
	Point p = GetRelease();
	GetNextMenu(p);
	current_menu_func();
}

void PastTrips()
{
	current_menu_val = PASTTRIPS;
	int i;
	int maxLogs;
	int maxEntries;
	Point p;

	//check that there is a log0
	logExists = checkIfLog();

	maxEntries = numEntries(logNum);

	//the first log will have to always be 0
	maxLogs = lastLog(0);

	//first time getting here we should set log to the most recent one
	if (initial == 1)
		logNum = maxLogs-1;

	printf("number of logs = %d\n", maxLogs);

	//cover up the "please insert sd card" if it's still there
	FilledRectangle(50,431,625,480, DARK_GREEN);

	int myFileHandle;
	char logname[20];

	snprintf(logname, 20, "log%d.txt", logNum);
	printf("logname in pasttrips = %s\n", logname);

	//cannot afford to be constantly opening and closing, so open just once here
	if((myFileHandle = alt_up_sd_card_fopen(logname, false)) != -1)
		printf("File opened for getting next, filehandle = %d\n", myFileHandle);
	else {
		//handle SD card not inserted
		printf("File not opened for next\n");
		Init_SDCard();
		PastTrips();
	}

	/* delay for debouncing screen */
	for(i=0; i<1000; i++);

	if (redraw==1) {
		DrawPastTrips(&redraw);
	}

	DrawTripData(maxLogs, myFileHandle, maxEntries, logNum, logExists);
	DrawPath(logNum);

	WaitForTouch();

	p = GetRelease();
	//this is here to make sure the SD card is inserted, otherwise program will crash
	Init_SDCard();
	PrevNext(p, 0, maxLogs);
	GetNextMenu(p);
	current_menu_func();
}

void NewTrip()
{
	char time[100];
	char latitude[100];
	char longitude[100];
	char altitude[100];
	//snprintf(time, 100,"4:20: ");
	//snprintf(latitude, 100, "S52* 26' 59.79\"");
	//snprintf(longitude, 100, "N13* 23' 36.11\"");
	//snprintf(altitude, 100, "1180.23m");

	Point p;
	/* update the menu state variable */
	current_menu_val = NEWTRIP;
	if(redraw)
	{

		/* set the screen background */
		FilledRectangle(0,0,800,480, DARK_GREEN);
		DrawButton(0, 0, 800, 40, "Logging trip data...", sizeof ("Logging trip data..."), WHITE, BLACK);

		/* draw data screen and position screen */
		DrawButton(50, 50, 400, 430, "", 0, BLACK, GRAY);
		DrawButton(400, 50, 750, 430, "", 0, BLACK, SADDLE_BROWN);
		VLine(400, 50, 380, BLACK);
		Triangle(565, 240, 585, 240, 575, 220, BLACK);
		Fill(575, 230, BLACK, BLACK);

		/* Back to main menu button */
		DrawButton(650, 430, 800, 480, "Main Menu", sizeof("Main Menu")-1, BLACK, BURLY_WOOD);

		/* draw the GPS data labels*/
		DrawGpsLabels();
		redraw = 0;
	}

	/* delay for debouncing screen */
	int i;
	for(i=0; i<10000; i++);

	char * gpsdat;

	char temp[256];

	//DEBUG
	int iteration = 0;
	int logNum = 0;
	int fileHandle;
	//DEBUG

	while(1)
	{
		//DEBUG
		printf("iteration = %d\n", iteration);
		int r2 = rand()%100;
		printf("r2 = %d\n", r2);
		int newlog = (iteration*2)/r2;

		if (newlog > 0) {
			logNum++;
			iteration = 0;
		}
		printf("log = %d\n", logNum);
		int r = rand() % 10;
		printf("random = %d\n", r);
		//DEBUG

		char temptime[256] = "Time: 3:3";
		char time[256];
		snprintf(time,256,"%s%d ",temptime,r);
		char templat[30] = "Latitude: 45.46";
		char lat[30];
		snprintf(lat,30,"%s%d ",templat,r);
		char templon[30] = "Longitude: 46.53";
		char lon[30];
		snprintf(lon,30,"%s%d ",templon,r);
		char tempalt[30] = "Altitude: 542";
		char alt[30];
		snprintf(alt,30,"%s%d ",tempalt,r);
		char gpsdat[500];
		snprintf(gpsdat,500,"%s%s%s%s",time,lat,lon,alt);

		strcpy(temp, gpsdat);
		printf("%s\n", temp);

		fileHandle = writeToSd(temp,logNum,sizeof(temp));
		while (fileHandle == -1) {
			printf("Trying again, re-init sd-card\n");
			Init_SDCard();
			strcpy(temp,gpsdat);
			fileHandle = writeToSd(temp,logNum,sizeof(temp));
		}

		/* DEBUG
		gpsdat = getGpsData();
		strcpy(temp, gpsdat);
		printf("%s\n", gpsdat);
		*/

		extractGpsTime(temp , time);
		//printf("%s\n", time);
		strcpy(temp, gpsdat);
		extractGpsLatitude(temp, latitude);
		//printf("%s\n", latitude);
		strcpy(temp, gpsdat);
		extractGpsLongitude(temp, longitude);
		//printf("%s\n", longitude);
		strcpy(temp, gpsdat);
		extractGpsAltitude(temp,  altitude);
		//printf("%s\n", altitude);

		char logname[20];
		sprintf(logname, "log%d", logNum);

		DrawGpsData(time, latitude, longitude, altitude, logname);

		iteration++;

		for(i=0; i<1000000; i++)
		{
			if(ScreenTouched())
			{
				goto touched;
			}
		}
	}
touched:

	p = GetRelease();
	GetNextMenu(p);
	current_menu_func();
}


