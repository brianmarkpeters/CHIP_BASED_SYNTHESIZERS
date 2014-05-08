// TIA SYNTHESIZER 1.00
// -Brian Peters
// www.brianpeters.net
//
// - This is the single file version of the program for ease of copy/pasting.
// - Please see the schematic, feature list, channel/CC guide, and video
//    for more information.
//
//
//
// - MATERIALS
//
//    - 1x Teensy 2.0 Board
//       (www.pjrc.com)
//    - 1x TIA 40 Pin DIP
//       (eBay or www.best-electronics-ca.com)
//    - 1x 2.0 MHz Full Can Oscillator
//       (eBay)
//    - 1x 10k Resistor
//    - 1x 10uF Electrolytic Capacitor
//    - 1x 1/4" Audio Jacks (or 3.5mm)
//    - 1x 60+ Row Breadboard
//    - 1x USB A to Mini B Cable
//    - Lots of Wire
//
//
// - UPLOADING PROGRAM TO TEENSY (Ideally before wiring everything.)
//    - After installing the teensy loader, go to
//       Tools > Board > Teensy 2.0
//          and
//       Tools > USB Type > MIDI
//    - Copy/Paste this program into the Arduino IDE
//    - Upload
//
//
// - CONNECTION LIST (TIA IS INDEXED STARTING AT PIN 1)
//
//    - Teensy Vcc
//       - Breadboard +5V Bus
//          - TIA PIN 20 (Vcc)
//          - TIA PIN 23 (CS1)
//          - TIA PIN 28 (A4)
//          - Oscillator +5V (top right with sharp edge on top left)
//          - 10k Resistor Leg 1
//    - Teensy GND
//       - Breadboard GND Bus
//          - TIA PIN 1 (Vss)
//          - TIA PIN 19 (D5)
//          - TIA PIN 21 (!CS3)
//          - TIA PIN 22 (!CS2)
//          - TIA PIN 25 (R/!W)
//          - TIA PIN 27 (A5)
//          - TIA PIN 33 (D6)
//          - TIA PIN 34 (D7)
//          - Oscillator GND (bottom left with sharp edge on top left)
//          - Audio Jack Sleeve
//    - Teensy B0
//       - TIA PIN 14 (D0)
//    - Teensy B1
//       - TIA PIN 15 (D1)
//    - Teensy B2
//       - TIA PIN 16 (D2)
//    - Teensy B3
//       - TIA PIN 17 (D3)
//    - Teensy B4
//       - TIA PIN 18 (D4)
//    - Teensy C6
//       - TIA PIN 24 (!CS0)
//    - Teensy D0
//       - TIA PIN 32 (A0)
//    - Teensy D1
//       - TIA PIN 31 (A1)
//    - Teensy D2
//       - TIA PIN 30 (A2)
//    - Teensy D3
//       - TIA PIN 29 (A3)
//    - Oscillator Signal Out (bottom right with sharp edge on top left)
//       - TIA PIN 11 (OSC)
//       - TIA PIN 26 (02)
//    - TIA PIN 12 (AUDIO0)
//       - 10k Resistor Leg 2
//       - 10uF Electrolytic Capacitor Positive Leg
//    - TIA PIN 13 (AUDIO1)
//       - 10k Resistor Leg 2
//       - 10uF Electrolytic Capacitor Positive Leg
//    - 10uF Electrolytic Capacitor Negative Leg
//       - Audio Jack Tip
//
//
//
// PROGRAM STRUCTURE:
//
//    Global Variables
//
//    setup
//    loop
//
//
//    AUXILIARY FUNCTIONS:
//       
//       mostRecentNoteInArray
//       eraseOldestNoteInArray
//       eraseOldestNoteInArraySmartPoly
//       mostRecentVelocityInArray
//       placeNoteInFirstEmptyArrayPosition
//       placeNoteAndVelocityInFirstEmptyArrayPosition
//       eraseThisNoteInArray
//       testArrayContentsForAtLeastTwoNotes
//       findSingleNoteInArray
//       testArrayContentsForNoNotes
//       testArrayContentsForNoNotesPoly
//       
//
//    OSCILLATOR AND ARPEGGIATOR FUNCTIONS:
//
//       dumbPolyEngine
//       smartPolyEngine
//       dumbLeadOneArpeggiator
//       dumbLeadOneEngine
//       smartLeadOneArpeggiator
//       smartLeadOneEngine
//       dumbLeadTwoArpeggiator
//       dumbLeadTwoEngine
//       smartLeadTwoArpeggiator
//       smartLeadTwoEngine    
//
//
//    MISC FUNCTIONS:
//
//       ledFlash
//
//
//    TIA FUNCTIONS:
//
//       writeNote
//       smartPolyWriteNote
//       writeAmplitude
//       writePolynomialSelect
//       pulseCS
//
//
//    MIDI CALLBACK FUNCTIONS:
//
//       OnNoteOn
//       OnNoteOff
//       OnControlChange
//       OnPitchChange
//
//
//
// The usbMIDI.read() in the main loop can trigger all of the MIDI callback
// functions, which update static variables in the oscillator and arpeggiator
// functions.
//
// All of the functions in the main loop have a 1 as their first argument,
// which allows them to update their portamento/arpeggiation/tremolo/decay.








//////////////////////////////////////////
//  GLOBAL VARIABLES
//////////////////////////////////////////


byte transferHoldTime = 1;
boolean noVelocity = HIGH; // Defaults all velocities to 127
byte CS0pinChip0 = 9;
byte bendRange = 12;






//////////////////////////////////////////
//  SETUP AND LOOP
//////////////////////////////////////////


void setup() {
   usbMIDI.setHandleNoteOff(OnNoteOff);
   usbMIDI.setHandleNoteOn(OnNoteOn) ;
   usbMIDI.setHandleControlChange(OnControlChange) ;
   usbMIDI.setHandlePitchChange(OnPitchChange);
   

   DDRB = DDRB | B00011111; 
   DDRD = DDRD | B00001111;
  
   pinMode(CS0pinChip0, OUTPUT);
   digitalWrite(CS0pinChip0,HIGH);
  
   // LED Pin
   pinMode(11, OUTPUT);
   
   ledFlash(0);

   for (byte i=1;i<3;i++) {
      for (byte j=0;j<1;j++) {
         writePolynomialSelect(4,i,j);
         writeAmplitude(127,i,j);
         for (byte k=33;k<65;k=k+2) {
            writeNote(k,i,j);
            digitalWrite(11,HIGH);
            delay(4);
            digitalWrite(11,LOW);
            delay(26);
         }
         writeAmplitude(0,i,j);
      }
   }

   for (byte i=1;i<3;i++) {
      for (byte j=0;j<1;j++) {
         writePolynomialSelect(9,i,j);
         writeAmplitude(127,i,j);
         for (byte k=33;k<65;k=k+4) {
            writeNote(k,i,j);
            digitalWrite(11,HIGH);
            delay(40);
            digitalWrite(11,LOW);
            delay(60);
         }
            writeAmplitude(0,i,j);
            writePolynomialSelect(4,i,j);
      }
   }
}




void loop(){
   usbMIDI.read();
   ledFlash(1);
   dumbPolyEngine(1,0,0);
   smartPolyEngine(1,0,0);
   dumbLeadOneArpeggiator(1,0,0,0,0);
   dumbLeadOneEngine(1,0,0,LOW,0,0);
   smartLeadOneArpeggiator(1,0,0,0,0);
   smartLeadOneEngine(1,0,0,LOW,0,0);
   dumbLeadTwoArpeggiator(1,0,0,0,0);
   dumbLeadTwoEngine(1,0,0,LOW,0,0);
   smartLeadTwoArpeggiator(1,0,0,0,0);
   smartLeadTwoEngine(1,0,0,LOW,0,0);
}






//////////////////////////////////////////
//  AUXILIARY FUNCTIONS
//////////////////////////////////////////

byte mostRecentNoteInArray (byte noteNumberArray[8], 
   unsigned long noteDurationArray[8]) {
  
   byte i = 0;
   unsigned long mostRecentNoteDuration = 0;
   byte mostRecentNote = 0;
  
   for (i=0;i<8;i++) {
      if (noteDurationArray[i] > mostRecentNoteDuration) {
         mostRecentNoteDuration = noteDurationArray[i];
         mostRecentNote = noteNumberArray[i];
      }
   }
   return mostRecentNote;
}




void eraseOldestNoteInArray (byte (&noteNumberArray)[6], 
   byte (&noteVelocityArray)[6], unsigned long (&noteDurationArray)[6]) {
  
   byte i = 0;
   unsigned long oldestDuration = 4294967295;
   byte oldestArrayPosition = 0;
  
   for (i=0;i<6;i++) {
      if (noteDurationArray[i] < oldestDuration) {
         oldestDuration = noteDurationArray[i];
         oldestArrayPosition = i;
      }
   }
   noteNumberArray[oldestArrayPosition] = 0;
   noteVelocityArray[oldestArrayPosition] = 0;
   noteDurationArray[oldestArrayPosition] = 0;
}



void eraseOldestNoteInArraySmartPoly (byte (&noteNumberArray)[2], 
   byte (&noteVelocityArray)[2], unsigned long (&noteDurationArray)[2]) {
   
   byte i = 0;
   unsigned long oldestDuration = 4294967295;
   byte oldestArrayPosition = 0;
  
   for (i=0;i<2;i++) {
      if (noteDurationArray[i] < oldestDuration) {
         oldestDuration = noteDurationArray[i];
         oldestArrayPosition = i;
      }
   }
   noteNumberArray[oldestArrayPosition] = 0;
   noteVelocityArray[oldestArrayPosition] = 0;
   noteDurationArray[oldestArrayPosition] = 0;
}



byte mostRecentVelocityInArray (byte noteVelocityArray[8], 
   unsigned long noteDurationArray[8]) {
  
   byte i = 0;
   unsigned long mostRecentNoteDuration = 0;
   byte mostRecentVelocity = 0;
  
   for (i=0;i<8;i++) {
      if (noteDurationArray[i] > mostRecentNoteDuration) {
         mostRecentNoteDuration = noteDurationArray[i];
         mostRecentVelocity = noteVelocityArray[i];
      }
   }
   return mostRecentVelocity;
}



void placeNoteInFirstEmptyArrayPosition(byte (&noteNumberArray)[8], 
   unsigned long (&noteDurationArray)[8], byte outsideNoteNumber) {
	
   boolean noteAlreadyStored = LOW;
	
   for (int arrayCounter = 0; arrayCounter < 8 ; arrayCounter++) {
      if (noteNumberArray[arrayCounter] == outsideNoteNumber) {
         noteAlreadyStored = HIGH;
         break;
      }
   }
   if (noteAlreadyStored == LOW) {
      for (int arrayCounter = 0; arrayCounter < 8; arrayCounter++) {
         if (noteNumberArray[arrayCounter] == 0) {
            noteNumberArray[arrayCounter] = outsideNoteNumber;
            noteDurationArray[arrayCounter] = millis();
            break;
         }
      }
   }
}



void placeNoteAndVelocityInFirstEmptyArrayPosition(byte (&noteNumberArray)[8], 
   unsigned long (&noteDurationArray)[8], byte outsideNoteNumber, 
   byte (&noteVelocityArray)[8], byte outsideNoteVelocity) {
	
   boolean noteAlreadyStored = LOW;
	
   for (int arrayCounter = 0; arrayCounter < 8 ; arrayCounter++) {
      if (noteNumberArray[arrayCounter] == outsideNoteNumber) {
         noteAlreadyStored = HIGH;
         break;
      }
   }
   if (noteAlreadyStored == LOW) {
      for (int arrayCounter = 0; arrayCounter < 8; arrayCounter++) {
         if (noteNumberArray[arrayCounter] == 0) {
            noteNumberArray[arrayCounter] = outsideNoteNumber;
            noteDurationArray[arrayCounter] = millis();
            noteVelocityArray[arrayCounter] = outsideNoteVelocity;
            break;
         }
      }
   }
}




void eraseThisNoteInArray(byte (&noteNumberArray)[8], 
   unsigned long (&noteDurationArray)[8], byte outsideNoteNumber) {
  
   for (int arrayCounter = 0; arrayCounter < 8; arrayCounter++) {
      if (noteNumberArray[arrayCounter] == outsideNoteNumber) {
         noteNumberArray[arrayCounter] = 0;
         noteDurationArray[arrayCounter] = 0;
      }
   }
}



boolean testArrayContentsForAtLeastTwoNotes (byte noteNumberArray[8]) {
   byte testArrayContents1 = 0;
   byte testArrayContents2 = 0;
   byte i = 0;
   byte j = 0;
		
   while (i < 8) {
      if (noteNumberArray[i] > 0) {
         testArrayContents1 = noteNumberArray[i];
         j = i+1;
         while (j < 8) {
            if (noteNumberArray[j] > 0) {
               testArrayContents2 = noteNumberArray[j];
               break;
            }
            j++;
         }
         break;
      }
      i++;
   }
   if ((testArrayContents1 > 0) && (testArrayContents2 > 0)) {
      return HIGH;
   }
   else {
      return LOW;
   }
}



byte findSingleNoteInArray (byte noteNumberArray[8]) {
  
   byte arrayContents = 0;
   byte i = 0;
  
   while (i < 8) {
      if (noteNumberArray[i] > 0) {
         arrayContents = noteNumberArray[i];
         break;
      }
      i++;
   }
   return arrayContents;
}



boolean testArrayContentsForNoNotes (byte noteNumberArray[8]) {
  
   byte testArrayContents1 = 0;
   byte testArrayContents2 = 0;
   byte i = 0;
   byte j = 0;
		
   while (i < 8) {
      if (noteNumberArray[i] > 0) {
         testArrayContents1 = noteNumberArray[i];
         j = i+1;
         while (j < 8) {
            if (noteNumberArray[j] > 0) {
               testArrayContents2 = noteNumberArray[j];
               break;
            }
            j++;
         }
         break;
      }
      i++;
   }
   if ((testArrayContents1 == 0) && (testArrayContents2 == 0)) {
      return HIGH;
   }
   else {
      return LOW;
   }
}



boolean testArrayContentsForNoNotesPoly (byte noteNumberArray[2]) {
  
   byte testArrayContents1 = 0;
   byte testArrayContents2 = 0;
   byte i = 0;
   byte j = 0;
    
   while (i < 2) {
      if (noteNumberArray[i] > 0) {
         testArrayContents1 = noteNumberArray[i];
         j = i+1;
         while (j < 2) {
            if (noteNumberArray[j] > 0) {
               testArrayContents2 = noteNumberArray[j];
               break;
            }
            j++;
         }
         break;
      }
      i++;
   }
   if ((testArrayContents1 == 0) && (testArrayContents2 == 0)) {
      return HIGH;
   }
   else {
      return LOW;
   }
}






//////////////////////////////////////////
//  OSCILLATOR AND ARPEGGIATOR FUNCTIONS
//////////////////////////////////////////


void dumbPolyEngine(byte dataType, byte noteNumber, byte noteVelocity) {
  
   static byte noteNumberArray[2];
   static byte noteVelocityArray[2];
   static unsigned long noteDurationsArray[2];
   static boolean insideRun = LOW;
   static boolean decayOn = LOW;
   static boolean arrayFull = LOW;
   static unsigned long decayInterval;
   static unsigned long previousDecayMicros;
   unsigned long currentMicros;
   static byte currentDecayLevel1;
   static byte currentDecayLevel2;
   static boolean tremOn = LOW;
   static int tremOffset = 0;
   static unsigned int tremInterval = 1000;
   static unsigned long previousTremMicros;
   unsigned long currentMillis;
   static float tremRateDivisor = 21.0; // Smaller is faster.
   static float tremAmount;
   static float tremAmount2;
   static int tremMaxDepth = 7;
   static byte insideCC1 = 0;
   static byte engineChipOne = 0;
   byte polynomialType;
   byte i;

   switch (dataType) {
      case 1: // Main Loop Run
         if ((decayOn == HIGH) && (insideRun == HIGH)) {
            currentMicros = micros();
            if ((currentMicros - previousTremMicros) >= tremInterval) {
               currentMillis = millis();
               tremAmount = sin(((float)currentMillis * 
                  1.0/(float)tremRateDivisor));
               tremAmount2 = tremAmount * (float)tremMaxDepth;
               tremOffset = tremAmount2 * ((float)insideCC1/127.0);
               previousTremMicros = micros();
            }
            currentMicros = micros();
            if ((currentMicros - previousDecayMicros) >= decayInterval) {
               if (currentDecayLevel1 > 0) {
                  currentDecayLevel1--;
                  writeAmplitude((constrain(currentDecayLevel1 + 
                     tremOffset,0,15)),1,engineChipOne);
               }
               else {
                  writeAmplitude(0,1,engineChipOne);
               }
               if (currentDecayLevel2 > 0) {
                  currentDecayLevel2--;
                  writeAmplitude((constrain(currentDecayLevel2 + 
                     tremOffset,0,15)),2,engineChipOne);            
               }
               else {
                  writeAmplitude(0,2,engineChipOne);
               }
               previousDecayMicros = micros();
            }
         }
        
         if ((decayOn == LOW) && (insideRun == HIGH)) {
            currentMicros = micros();
            if ((currentMicros - previousTremMicros) >= tremInterval) {
               currentMillis = millis();
               tremAmount = sin(((float)currentMillis * 
                  1.0/(float)tremRateDivisor));
               tremAmount2 = tremAmount * (float)tremMaxDepth;
               tremOffset = tremAmount2 * ((float)insideCC1/127.0);
               if (currentDecayLevel1 > 0) {
                  writeAmplitude((constrain(currentDecayLevel1 + 
                     tremOffset,0,127)),1,engineChipOne);
               }
               if (currentDecayLevel2 > 0) {
                  writeAmplitude((constrain(currentDecayLevel2 + 
                     tremOffset,0,127)),2,engineChipOne);
               }
               previousTremMicros = micros();
            }
         }
      break;
    
      case 2: // Note On
         insideRun = HIGH;
         arrayFull = HIGH;
         for (i=0;i<2;i++) {
            if (noteNumberArray[i] == 0) {
               arrayFull = LOW;
            }
         }
         if (arrayFull == HIGH) {
            eraseOldestNoteInArraySmartPoly(noteNumberArray,
               noteVelocityArray,noteDurationsArray);    
         }
      
         for (i=0;i<2;i++) {
            if ((noteNumberArray[i] == 0) && (noteNumber > 0)) {
               noteNumberArray[i] = noteNumber;
               noteVelocityArray[i] = noteVelocity;
               noteDurationsArray[i] = millis();
                    
               switch (i) {
                  case 0: 
                     writeNote(noteNumber,1,engineChipOne);
                     writeAmplitude((noteVelocity >> 3),1,engineChipOne);
                     currentDecayLevel1 = (noteVelocity >> 3);
                  break;
               
                  case 1: 
                     writeNote(noteNumber,2,engineChipOne);
                     writeAmplitude((noteVelocity >> 3),2,engineChipOne);
                     currentDecayLevel2 = (noteVelocity >> 3);
                  break;
               }
               break;
            }
         } 
      break;
    
      case 3: // Note Off
         for (i=0;i<2;i++) {
            if ((noteNumberArray[i] == noteNumber) && (noteNumber > 0)) {
               noteNumberArray[i] = 0;
               noteVelocityArray[i] = 0;
               noteDurationsArray[i] = 0;
          
               switch (i) {
                  case 0: 
                     writeAmplitude(0,1,engineChipOne);
                     currentDecayLevel1 = 0;
                  break;
                  
                  case 1: 
                     writeAmplitude(0,2,engineChipOne);
                     currentDecayLevel2 = 0;
                  break;
               }
               if (testArrayContentsForNoNotesPoly(noteNumberArray) == HIGH) {
                  writeAmplitude(0,1,engineChipOne);
                  writeAmplitude(0,2,engineChipOne);
                  insideRun = LOW;
               }
               break;
            }
         }
      break;
    
      case 4: // Pitch Bend (Unused)
      break;
    
      case 5: // CC 1 Change
         insideCC1 = noteNumber;
      break;
    
      case 6: // CC3 Decay
         if ((noteNumber == 0) && (decayOn == HIGH)) {
            decayOn = LOW;
         }
         else if (noteNumber > 0) {
            decayOn = HIGH;
            decayInterval = map(noteNumber,0,127,100000,500);
         }
      break;

      case 7: // Polynomail Type Select
         polynomialType = map(noteNumber,0,127,0,15);
         writePolynomialSelect(polynomialType,1,engineChipOne);
         writePolynomialSelect(polynomialType,2,engineChipOne);
      break;
   }
}



void smartPolyEngine(byte dataType, byte noteNumber, byte noteVelocity) {

   static byte noteNumberArray[2];
   static byte noteVelocityArray[2];
   static unsigned long noteDurationsArray[2];
   static boolean insideRun = LOW;
   static boolean decayOn = LOW;
   static boolean arrayFull = LOW;
   static unsigned long decayInterval;
   static unsigned long previousDecayMicros;
   unsigned long currentMicros;
   static byte currentDecayLevel1;
   static byte currentDecayLevel2;
   // static byte currentDecayLevel3;
   // static byte currentDecayLevel4;
   // static byte currentDecayLevel5;
   // static byte currentDecayLevel6;
   static boolean tremOn = LOW;
   static int tremOffset = 0;
   static unsigned int tremInterval = 1000;
   static unsigned long previousTremMicros;
   unsigned long currentMillis;
   static float tremRateDivisor = 21.0; // Smaller is faster.
   static float tremAmount;
   static float tremAmount2;
   static int tremMaxDepth = 7;
   static byte insideCC1 = 0;
   static byte engineChipOne = 0;
   byte i;
   
   switch (dataType) {
      case 1: // Main Loop Run
         if ((decayOn == HIGH) && (insideRun == HIGH)) {
            currentMicros = micros();
            if ((currentMicros - previousTremMicros) >= tremInterval) {
               currentMillis = millis();
               tremAmount = sin(((float)currentMillis * 
                  1.0/(float)tremRateDivisor));
               tremAmount2 = tremAmount * (float)tremMaxDepth;
               tremOffset = tremAmount2 * ((float)insideCC1/127.0);
               previousTremMicros = micros();
            }
            currentMicros = micros();
            if ((currentMicros - previousDecayMicros) >= decayInterval) {
               if (currentDecayLevel1 > 0) {
                  currentDecayLevel1--;
                  writeAmplitude((constrain(currentDecayLevel1 + 
                     tremOffset,0,15)),1,engineChipOne);
               }
               else {
                  writeAmplitude(0,1,engineChipOne);
               }
               if (currentDecayLevel2 > 0) {
                  currentDecayLevel2--;
                  writeAmplitude((constrain(currentDecayLevel2 + 
                     tremOffset,0,15)),2,engineChipOne);            
               }
               else {
                  writeAmplitude(0,2,engineChipOne);
               }
               previousDecayMicros = micros();
            }
         }
        
         if ((decayOn == LOW) && (insideRun == HIGH)) {
            currentMicros = micros();
            if ((currentMicros - previousTremMicros) >= tremInterval) {
               currentMillis = millis();
               tremAmount = sin(((float)currentMillis * 
                  1.0/(float)tremRateDivisor));
               tremAmount2 = tremAmount * (float)tremMaxDepth;
               tremOffset = tremAmount2 * ((float)insideCC1/127.0);
               if (currentDecayLevel1 > 0) {
                  writeAmplitude((constrain(currentDecayLevel1 + 
                     tremOffset,0,127)),1,engineChipOne);
               }
               if (currentDecayLevel2 > 0) {
                  writeAmplitude((constrain(currentDecayLevel2 + 
                     tremOffset,0,127)),2,engineChipOne);
               }
               previousTremMicros = micros();
            }
         }
      break;
    
      case 2: // Note On
         insideRun = HIGH;
         arrayFull = HIGH;
         for (i=0;i<2;i++) {
            if (noteNumberArray[i] == 0) {
               arrayFull = LOW;
            }
         }
         if (arrayFull == HIGH) {
            eraseOldestNoteInArraySmartPoly(noteNumberArray,
               noteVelocityArray,noteDurationsArray);    
         }
         for (i=0;i<2;i++) {
            if ((noteNumberArray[i] == 0) && (noteNumber > 0)) {
               noteNumberArray[i] = noteNumber;
               noteVelocityArray[i] = noteVelocity;
               noteDurationsArray[i] = millis();          
               switch (i) {
                  case 0: 
                     smartPolyWriteNote(noteNumber,1,engineChipOne);
                     writeAmplitude((noteVelocity >> 3),1,engineChipOne);
                     currentDecayLevel1 = (noteVelocity >> 3);
                  break;
                  
                  case 1: 
                     smartPolyWriteNote(noteNumber,2,engineChipOne);
                     writeAmplitude((noteVelocity >> 3),2,engineChipOne);
                     currentDecayLevel2 = (noteVelocity >> 3);
                  break;
               }
               break;
            }
         } 
      break;
    
      case 3: // Note Off
         for (i=0;i<2;i++) {
            if ((noteNumberArray[i] == noteNumber) && (noteNumber > 0)) {
               noteNumberArray[i] = 0;
               noteVelocityArray[i] = 0;
               noteDurationsArray[i] = 0;
               switch (i) {
                  case 0: 
                     writeAmplitude(0,1,engineChipOne);
                     currentDecayLevel1 = 0;
                  break;
                  
                  case 1: 
                     writeAmplitude(0,2,engineChipOne);
                     currentDecayLevel2 = 0;
                  break;
               }
               if (testArrayContentsForNoNotesPoly(noteNumberArray) == HIGH) {
                  writeAmplitude(0,1,engineChipOne);
                  writeAmplitude(0,2,engineChipOne);
                  insideRun = LOW;
               }
               break;
            }
         }
      break;
    
      case 4: // Pitch Bend (Unused)
      break;
    
      case 5: // CC 1 Change
         insideCC1 = noteNumber;
      break;
    
      case 6: // CC3 Decay
         if ((noteNumber == 0) && (decayOn == HIGH)) {
            decayOn = LOW;
         }
         else if (noteNumber > 0) {
            decayOn = HIGH;
            decayInterval = map(noteNumber,0,127,100000,500);
         }
      break;
   }
}





void dumbLeadOneArpeggiator(byte dataType, byte noteNumber, byte noteVelocity, 
   byte noteNumberFromArray, unsigned long noteDurationFromArray) {
  
   static byte arpNoteNumberArray[8];
   static unsigned long arpNoteDurationArray[8];
   static boolean arpeggiatorOn;
   unsigned long currentMillis;
   static unsigned long previousMillis;
   unsigned long elapsedTimeInMillis;
   static unsigned int arpeggiatorInterval = 20;
   static byte currentArpArrayPosition;
   static boolean arpeggiatorRunning = LOW;
   byte transitionNote;

   switch (dataType) {
      case 1: // Main Loop Run
         if ((arpeggiatorOn == HIGH) && (arpeggiatorRunning == HIGH)) {
            currentMillis = millis();
            elapsedTimeInMillis = currentMillis - previousMillis;
            if (elapsedTimeInMillis >= arpeggiatorInterval) {
               dumbLeadOneEngine(3,arpNoteNumberArray[currentArpArrayPosition],
                  0,HIGH,1,0);
               if (currentArpArrayPosition < 7) {
                  currentArpArrayPosition++;
               }
               else {
                  currentArpArrayPosition = 0;
               }
               while (arpNoteNumberArray[currentArpArrayPosition] == 0) {
                  if (currentArpArrayPosition < 7) {
                     currentArpArrayPosition++;
                  }
                  else {
                     currentArpArrayPosition = 0;
                  }
               }
               dumbLeadOneEngine(2,arpNoteNumberArray[currentArpArrayPosition],
                  0,HIGH,1,0);
               previousMillis = millis();
            }
         }
      break;
    
      case 2: // Note On
         if (arpeggiatorOn == HIGH) {
            placeNoteInFirstEmptyArrayPosition(arpNoteNumberArray,
               arpNoteDurationArray,noteNumber);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
            if (arpeggiatorRunning == LOW) {
               dumbLeadOneEngine(2,noteNumber,0,HIGH,0,0);
            }
         }
      break;
    
      case 3: // Note Off
         if (arpeggiatorOn == HIGH) {
            eraseThisNoteInArray(arpNoteNumberArray,
               arpNoteDurationArray,noteNumber);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
            if (arpeggiatorRunning == LOW) {
               dumbLeadOneEngine(3,noteNumber,0,HIGH,0,0);
               if (testArrayContentsForNoNotes(arpNoteNumberArray) == LOW) {
                  dumbLeadOneEngine(2,findSingleNoteInArray(arpNoteNumberArray),
                     0,HIGH,0,0);
               }
            }
         }
      break;
    
      case 4: // CC4: Arpeggiator On/Off
         if ((noteNumber > 0) && (arpeggiatorOn == LOW)) {
            arpeggiatorOn = HIGH;
            dumbLeadOneEngine(8,0,0,HIGH,0,0);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
         }
         if (noteNumber == 0) {
            arpeggiatorOn = LOW;
            dumbLeadOneEngine(3,
               arpNoteNumberArray[currentArpArrayPosition],0,HIGH,0,0);
            transitionNote = 
               mostRecentNoteInArray(arpNoteNumberArray,arpNoteDurationArray);
            if (transitionNote > 0) {
               dumbLeadOneEngine(2,transitionNote,0,HIGH,0,0);
            }
            for (int i=0;i<8;i++) {
               dumbLeadOneEngine(8,i,0,LOW,
                  arpNoteNumberArray[i],arpNoteDurationArray[i]);
            }
         }
      break;
    
      case 5: // Changes Arpeggiator Speed
         arpeggiatorInterval = map(noteNumber,0,127,20,200);
      break;
    
      case 6: // Accepts noteNumberArray and noteDurationArray
         arpNoteNumberArray[noteNumber] = noteNumberFromArray;
         arpNoteDurationArray[noteNumber] = noteDurationFromArray;
      break;
   }
}




void dumbLeadOneEngine(byte dataType, byte noteNumber, 
   float floatData, boolean noteComingFromArpeggiator, 
   byte arpNoteNumberFromArray, unsigned long arpNoteDurationFromArray) {

   static boolean insideRun = LOW;
   static byte noteNumberArray[8];
   static unsigned long noteDurationArray[8];
   static boolean portamentoOn = HIGH;
   static boolean portamentoRunning = LOW;
   static boolean portamentoDirection;
   unsigned long currentMicros;
   static unsigned long previousPortamentoChange;
   static unsigned long portamentoSpeedInMicros = 10000;
   static byte portamentoStepInNotes = 1;
   static int bendMod = 0;
   static byte insideNoteNumber;
   static byte insideTargetNoteNumber;
   static boolean arpeggiatorOn = LOW;
   static unsigned long decayInterval;
   static boolean decayOn = LOW;
   static byte currentDecayLevel = 0;
   static unsigned long previousDecayMicros;
   static byte engineChannel = 1;
   static byte engineChip = 0;
   static byte modwheelAmplitude = 15;
   byte insideNoteNumberAfterMods;
   static byte polynomialType = 4;
  
   switch (dataType) {
      case 1: // Main Loop Run
         if ((insideRun == HIGH) && (portamentoOn == HIGH) && 
            (portamentoRunning == HIGH) && (decayOn == LOW)) {
            currentMicros = micros();
            if ((portamentoDirection == LOW) && (currentMicros - 
               previousPortamentoChange >= portamentoSpeedInMicros)) {
            
               insideNoteNumber = insideNoteNumber - portamentoStepInNotes;
               previousPortamentoChange = micros();
               if (insideNoteNumber <= insideTargetNoteNumber) {
                  portamentoRunning = LOW;
                  insideNoteNumber = insideTargetNoteNumber;
               }
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
            }
            if ((portamentoDirection == HIGH) && (currentMicros - 
               previousPortamentoChange >= portamentoSpeedInMicros)) {
            
               insideNoteNumber = insideNoteNumber + portamentoStepInNotes;
               previousPortamentoChange = micros();
               if (insideNoteNumber >= insideTargetNoteNumber) {
                  portamentoRunning = LOW;
                  insideNoteNumber = insideTargetNoteNumber;
               }
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
            }
         }
         if ((decayOn == HIGH) && (insideRun == HIGH)) {
            currentMicros = micros();
            if ((currentMicros - previousDecayMicros) >= decayInterval) {
               writeAmplitude((modwheelAmplitude - currentDecayLevel),
                  engineChannel,engineChip);
               if (currentDecayLevel < 15) {
                  currentDecayLevel++;
               }
               previousDecayMicros = micros();
            }
         }
      break;
    
      case 2: // Note On
         if ((arpeggiatorOn == LOW) || (noteComingFromArpeggiator == HIGH)) {
            if (insideRun == LOW) {
               insideNoteNumber = noteNumber;
               insideTargetNoteNumber = noteNumber;
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               writePolynomialSelect(polynomialType,engineChannel,engineChip);
               writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
               writeAmplitude(modwheelAmplitude,engineChannel,engineChip);
               if (arpeggiatorOn == LOW) {
                  placeNoteInFirstEmptyArrayPosition(noteNumberArray,
                     noteDurationArray,noteNumber);
               }
               insideRun = HIGH;
               portamentoRunning = LOW;
            }
      
            else if ((insideRun == HIGH) && (arpeggiatorOn == LOW)) {
               insideTargetNoteNumber = noteNumber;
               portamentoRunning = HIGH;
               placeNoteInFirstEmptyArrayPosition(noteNumberArray,
                  noteDurationArray,noteNumber);
               if (insideTargetNoteNumber > insideNoteNumber) {
                  portamentoDirection = HIGH;
               }
               else {
                  portamentoDirection = LOW;
               }
            }
            currentDecayLevel = 0;
      
            if ((decayOn == HIGH) && (arpeggiatorOn == LOW)) {
               insideNoteNumber = insideTargetNoteNumber;
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
            }
         }
      break;
      
      case 3: // Note Off
         if ((arpeggiatorOn == LOW) || (noteComingFromArpeggiator == HIGH)) {
            if (arpeggiatorOn == LOW) {
               eraseThisNoteInArray(noteNumberArray,noteDurationArray,noteNumber);
               if (mostRecentNoteInArray(noteNumberArray,noteDurationArray) == 0) {
                  writeAmplitude(0,engineChannel,engineChip);
                  insideRun = LOW;
               }
               else if (decayOn == LOW) {
                  dumbLeadOneEngine(2,mostRecentNoteInArray(noteNumberArray,
                     noteDurationArray),0,LOW,0,0);
               }
            }
            if ((arpeggiatorOn == HIGH) && (arpNoteNumberFromArray == 0)) {
               writeAmplitude(0,engineChannel,engineChip);
               insideRun = LOW;
            }
            // If notes are coming from main arp sequencer.
            if ((arpeggiatorOn == HIGH) && (arpNoteNumberFromArray == 1)) { 
               insideRun = LOW;
            }
         }
      break;
    
      case 4: // Pitch Bend
         if (floatData == 8192) {
            bendMod = 0;
         }
         if (floatData < 8192) {
            bendMod = map(bendRange,1,12,0,-31);
            bendMod = map(floatData,0,8191,bendMod,0);
         }
         if (floatData > 8192) {
            bendMod = map(bendRange,1,12,0,31);
            bendMod = map(floatData,8193,16383,0,bendMod); 
         }
         insideNoteNumberAfterMods = insideNoteNumber + bendMod;
         writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
      break;
    
      case 5: // ModWheel
         modwheelAmplitude = map(noteNumber,0,127,0,15);
         if ((insideRun == HIGH) && (decayOn == LOW)) {
            writeAmplitude(modwheelAmplitude,engineChannel,engineChip);
         }
      break;
    
      case 6: // CC3 Decay
         if ((noteNumber == 0) && (decayOn == HIGH)) {
            decayOn = LOW;
            portamentoOn = HIGH;
         }
         else {
            decayOn = HIGH;
            portamentoOn = LOW;
            decayInterval = map(noteNumber,0,127,100000,500);
         }
      break;
    
      case 7: // CC5 Arp Speed Change
      break;
    
      case 8: // Arpeggiator Has Turned On Or Off.  Receive or transmit array.
         if (noteComingFromArpeggiator == HIGH) {
            arpeggiatorOn = HIGH;
            for (int i=0;i<8;i++) {
               dumbLeadOneArpeggiator(6,i,0,noteNumberArray[i],
                  noteDurationArray[i]);
            }
         }
         else {
            arpeggiatorOn = LOW;
            noteNumberArray[noteNumber] = arpNoteNumberFromArray;
            noteDurationArray[noteNumber] = arpNoteDurationFromArray;
         }
      break;
    
      case 9: // Polynomail Type Select
         polynomialType = map(noteNumber,0,127,0,15);
         writePolynomialSelect(polynomialType,engineChannel,engineChip);
      break;

      case 10: // Porta speed
         portamentoStepInNotes = map(noteNumber,0,127,1,10);
      break;
   }
}




void smartLeadOneArpeggiator(byte dataType, byte noteNumber, 
   byte noteVelocity, byte noteNumberFromArray, 
   unsigned long noteDurationFromArray) {

   static byte arpNoteNumberArray[8];
   static unsigned long arpNoteDurationArray[8];
   static boolean arpeggiatorOn;
   unsigned long currentMillis;
   static unsigned long previousMillis;
   unsigned long elapsedTimeInMillis;
   static unsigned int arpeggiatorInterval = 20;
   static byte currentArpArrayPosition;
   static boolean arpeggiatorRunning = LOW;
   byte transitionNote;

   switch (dataType) {
      case 1: // Main Loop Run
         if ((arpeggiatorOn == HIGH) && (arpeggiatorRunning == HIGH)) {
            currentMillis = millis();
            elapsedTimeInMillis = currentMillis - previousMillis;
            if (elapsedTimeInMillis >= arpeggiatorInterval) {
               smartLeadOneEngine(3,
                  arpNoteNumberArray[currentArpArrayPosition],0,HIGH,1,0);
               if (currentArpArrayPosition < 7) {
                  currentArpArrayPosition++;
               }
               else {
                  currentArpArrayPosition = 0;
               }
               while (arpNoteNumberArray[currentArpArrayPosition] == 0) {
                  if (currentArpArrayPosition < 7) {
                     currentArpArrayPosition++;
                  }
                  else {
                     currentArpArrayPosition = 0;
                  }
               }
               smartLeadOneEngine(2,
                  arpNoteNumberArray[currentArpArrayPosition],0,HIGH,1,0);
               previousMillis = millis();
            }
         }
      break;
    
      case 2: // Note On
         if (arpeggiatorOn == HIGH) {
            placeNoteInFirstEmptyArrayPosition(arpNoteNumberArray,
               arpNoteDurationArray,noteNumber);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
            if (arpeggiatorRunning == LOW) {
               smartLeadOneEngine(2,noteNumber,0,HIGH,0,0);
            }
         }
      break;
    
      case 3: // Note Off
         if (arpeggiatorOn == HIGH) {
            eraseThisNoteInArray(arpNoteNumberArray,
               arpNoteDurationArray,noteNumber);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
            if (arpeggiatorRunning == LOW) {
               smartLeadOneEngine(3,noteNumber,0,HIGH,0,0);
               if (testArrayContentsForNoNotes(arpNoteNumberArray) == LOW){
                  smartLeadOneEngine(2,
                     findSingleNoteInArray(arpNoteNumberArray),0,HIGH,0,0);
               }
            }
         }
      break;
    
      case 4: // CC4: Arpeggiator On/Off
         if ((noteNumber > 0) && (arpeggiatorOn == LOW)) {
            arpeggiatorOn = HIGH;
            smartLeadOneEngine(8,0,0,HIGH,0,0);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
         }
         if (noteNumber == 0) {
            arpeggiatorOn = LOW;
            smartLeadOneEngine(3,
               arpNoteNumberArray[currentArpArrayPosition],0,HIGH,0,0);
            transitionNote = mostRecentNoteInArray(
               arpNoteNumberArray,arpNoteDurationArray);
            if (transitionNote > 0) {
               smartLeadOneEngine(2,transitionNote,0,HIGH,0,0);
            }
            for (int i=0;i<8;i++) {
               smartLeadOneEngine(8,i,0,LOW,
                  arpNoteNumberArray[i],arpNoteDurationArray[i]);
            }
         }
      break;
    
      case 5: // Changes Arpeggiator Speed
         arpeggiatorInterval = map(noteNumber,0,127,20,200);
      break;
    
      case 6: // Accepts noteNumberArray and noteDurationArray
         arpNoteNumberArray[noteNumber] = noteNumberFromArray;
         arpNoteDurationArray[noteNumber] = noteDurationFromArray;
      break;
   }
}



void smartLeadOneEngine(byte dataType, byte noteNumber, float floatData, 
   boolean noteComingFromArpeggiator, byte arpNoteNumberFromArray, 
   unsigned long arpNoteDurationFromArray) {

   static boolean insideRun = LOW;
   static byte noteNumberArray[8];
   static unsigned long noteDurationArray[8];
   static boolean portamentoOn = HIGH;
   static boolean portamentoRunning = LOW;
   static boolean portamentoDirection;
   unsigned long currentMicros;
   static unsigned long previousPortamentoChange;
   static unsigned long portamentoSpeedInMicros = 10000;
   static byte portamentoStepInNotes = 1;
   static int bendMod = 0;
   static byte insideNoteNumber;
   static byte insideTargetNoteNumber;
   static boolean arpeggiatorOn = LOW;
   static unsigned long decayInterval;
   static boolean decayOn = LOW;
   static byte currentDecayLevel = 0;
   static unsigned long previousDecayMicros;
   static byte engineChannel = 1;
   static byte engineChip = 0;
   static byte modwheelAmplitude = 15;
   byte insideNoteNumberAfterMods;
   static byte polynomialType = 4;
  
   switch (dataType) {
    
      case 1: // Main Loop Run
         if ((insideRun == HIGH) && (portamentoOn == HIGH) && 
            (portamentoRunning == HIGH) && (decayOn == LOW)){
            
            currentMicros = micros();
            if ((portamentoDirection == LOW) && (currentMicros - 
               previousPortamentoChange >= portamentoSpeedInMicros)) {
               
               insideNoteNumber = insideNoteNumber - portamentoStepInNotes;
               previousPortamentoChange = micros();
               if (insideNoteNumber <= insideTargetNoteNumber) {
                  portamentoRunning = LOW;
                  insideNoteNumber = insideTargetNoteNumber;
               }
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               smartPolyWriteNote(insideNoteNumberAfterMods,engineChannel,engineChip);
            }
            if ((portamentoDirection == HIGH) && (currentMicros - 
               previousPortamentoChange >= portamentoSpeedInMicros)) {
               
               insideNoteNumber = insideNoteNumber + portamentoStepInNotes;
               previousPortamentoChange = micros();
               if (insideNoteNumber >= insideTargetNoteNumber) {
                  portamentoRunning = LOW;
                  insideNoteNumber = insideTargetNoteNumber;
               }
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               smartPolyWriteNote(insideNoteNumberAfterMods,
                  engineChannel,engineChip);
            }
         }
         if ((decayOn == HIGH) && (insideRun == HIGH)) {
            currentMicros = micros();
            if ((currentMicros - previousDecayMicros) >= decayInterval) {
               writeAmplitude((modwheelAmplitude - currentDecayLevel),
                  engineChannel,engineChip);
               if (currentDecayLevel < 15) {
                  currentDecayLevel++;
               }
               previousDecayMicros = micros();
            }
         }
      break;
      
   
   
   
      case 2: // Note On
         if ((arpeggiatorOn == LOW) || (noteComingFromArpeggiator == HIGH)) {
            if (insideRun == LOW) {
               insideNoteNumber = noteNumber;
               insideTargetNoteNumber = noteNumber;
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               smartPolyWriteNote(insideNoteNumberAfterMods,
                  engineChannel,engineChip);
               writeAmplitude(modwheelAmplitude,engineChannel,engineChip);
               if (arpeggiatorOn == LOW) {
                  placeNoteInFirstEmptyArrayPosition(noteNumberArray,
                     noteDurationArray,noteNumber);
               }
               insideRun = HIGH;
               portamentoRunning = LOW;
            }
         
            else if ((insideRun == HIGH) && (arpeggiatorOn == LOW)) {
               insideTargetNoteNumber = noteNumber;
               portamentoRunning = HIGH;
               placeNoteInFirstEmptyArrayPosition(noteNumberArray,
                  noteDurationArray,noteNumber);
               if (insideTargetNoteNumber > insideNoteNumber) {
                  portamentoDirection = HIGH;
               }
               else {
                  portamentoDirection = LOW;
               }
            }
            currentDecayLevel = 0;
         
            if ((decayOn == HIGH) && (arpeggiatorOn == LOW)) {
               insideNoteNumber = insideTargetNoteNumber;
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               smartPolyWriteNote(insideNoteNumberAfterMods,
                  engineChannel,engineChip);
            }
         }
      break;
         
      case 3: // Note Off
         if ((arpeggiatorOn == LOW) || (noteComingFromArpeggiator == HIGH)) {
            if (arpeggiatorOn == LOW) {
               eraseThisNoteInArray(noteNumberArray,noteDurationArray,noteNumber);
               
               if (mostRecentNoteInArray(noteNumberArray,noteDurationArray) == 0) {
                  writeAmplitude(0,engineChannel,engineChip);
                  insideRun = LOW;
               }
               else if (decayOn == LOW) {
                  smartLeadOneEngine(2,mostRecentNoteInArray(
                     noteNumberArray,noteDurationArray),0,LOW,0,0);
               }
            }
         
            if ((arpeggiatorOn == HIGH) && (arpNoteNumberFromArray == 0)) {
               writeAmplitude(0,engineChannel,engineChip);
               insideRun = LOW;
            }
   
            // If notes are coming from main arp sequencer.      
            if ((arpeggiatorOn == HIGH) && (arpNoteNumberFromArray == 1)) {
               insideRun = LOW;
            }
         }
      break;
       
      case 4: // Pitch Bend
         if (floatData == 8192) {
            bendMod = 0;
         }
         if (floatData < 8192) {
            bendMod = map(floatData,0,8191,((-1) * bendRange),0);
         }
         if (floatData > 8192) {
            bendMod = map(floatData,8193,16383,0,bendRange); 
         }
         insideNoteNumberAfterMods = insideNoteNumber + bendMod;
         smartPolyWriteNote(insideNoteNumberAfterMods,engineChannel,engineChip);
      break;
       
      case 5: // ModWheel
         modwheelAmplitude = map(noteNumber,0,127,0,15);
            if ((insideRun == HIGH) && (decayOn == LOW)) {
               writeAmplitude(modwheelAmplitude,engineChannel,engineChip);
            }
      break;
       
      case 6: // CC3 Decay
         if ((noteNumber == 0) && (decayOn == HIGH)) {
            decayOn = LOW;
            portamentoOn = HIGH;
         }
         else {
            decayOn = HIGH;
            portamentoOn = LOW;
            decayInterval = map(noteNumber,0,127,100000,500);
         }
      break;
       
      case 7: // CC5 Arp Speed Change
      break;
       
      case 8: // Arpeggiator Has Turned On Or Off.  Receive or transmit array.
         if (noteComingFromArpeggiator == HIGH) {
            arpeggiatorOn = HIGH;
            for (int i=0;i<8;i++) {
               smartLeadOneArpeggiator(6,i,0,noteNumberArray[i],
                  noteDurationArray[i]);
            }
         }
         
         else {
            arpeggiatorOn = LOW;
            noteNumberArray[noteNumber] = arpNoteNumberFromArray;
            noteDurationArray[noteNumber] = arpNoteDurationFromArray;
         }
      break;
      
      case 10: // Porta speed
         portamentoStepInNotes = map(noteNumber,0,127,1,10);
      break;
   }
}










void dumbLeadTwoArpeggiator(byte dataType, byte noteNumber, 
   byte noteVelocity, byte noteNumberFromArray, 
   unsigned long noteDurationFromArray) {

   static byte arpNoteNumberArray[8];
   static unsigned long arpNoteDurationArray[8];
   static boolean arpeggiatorOn;
   unsigned long currentMillis;
   static unsigned long previousMillis;
   unsigned long elapsedTimeInMillis;
   static unsigned int arpeggiatorInterval = 20;
   static byte currentArpArrayPosition;
   static boolean arpeggiatorRunning = LOW;
   byte transitionNote;

   switch (dataType) {
      case 1: // Main Loop Run
         if ((arpeggiatorOn == HIGH) && (arpeggiatorRunning == HIGH)) {
            currentMillis = millis();
            elapsedTimeInMillis = currentMillis - previousMillis;
            if (elapsedTimeInMillis >= arpeggiatorInterval) {
               dumbLeadTwoEngine(3,arpNoteNumberArray[currentArpArrayPosition],
                  0,HIGH,1,0);
               if (currentArpArrayPosition < 7) {
                  currentArpArrayPosition++;
               }
               else {
                  currentArpArrayPosition = 0;
               }
               while (arpNoteNumberArray[currentArpArrayPosition] == 0) {
                  if (currentArpArrayPosition < 7) {
                     currentArpArrayPosition++;
                  }
                  else {
                     currentArpArrayPosition = 0;
                  }
               }
               dumbLeadTwoEngine(2,arpNoteNumberArray[currentArpArrayPosition],
                  0,HIGH,1,0);
               previousMillis = millis();
            }
         }
      break;
    
      case 2: // Note On
         if (arpeggiatorOn == HIGH) {
            placeNoteInFirstEmptyArrayPosition(arpNoteNumberArray,
               arpNoteDurationArray,noteNumber);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
            if (arpeggiatorRunning == LOW) {
               dumbLeadTwoEngine(2,noteNumber,0,HIGH,0,0);
            }
         }
      break;
    
      case 3: // Note Off
         if (arpeggiatorOn == HIGH) {
            eraseThisNoteInArray(arpNoteNumberArray,
               arpNoteDurationArray,noteNumber);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
            if (arpeggiatorRunning == LOW) {
               dumbLeadTwoEngine(3,noteNumber,0,HIGH,0,0);
               if (testArrayContentsForNoNotes(arpNoteNumberArray) == LOW) {
                  dumbLeadTwoEngine(2,findSingleNoteInArray(
                     arpNoteNumberArray),0,HIGH,0,0);
               }
            }
         }
      break;
    
      case 4: // CC4: Arpeggiator On/Off
         if ((noteNumber > 0) && (arpeggiatorOn == LOW)) {
            arpeggiatorOn = HIGH;
            dumbLeadTwoEngine(8,0,0,HIGH,0,0);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
         }
         if (noteNumber == 0) {
            arpeggiatorOn = LOW;
            dumbLeadTwoEngine(3,arpNoteNumberArray[currentArpArrayPosition],
               0,HIGH,0,0);
            transitionNote = mostRecentNoteInArray(arpNoteNumberArray,
               arpNoteDurationArray);
            if (transitionNote > 0) {
               dumbLeadTwoEngine(2,transitionNote,0,HIGH,0,0);
            }
            for (int i=0;i<8;i++) {
               dumbLeadTwoEngine(8,i,0,LOW,arpNoteNumberArray[i],
                  arpNoteDurationArray[i]);
            }
         }
      break;
    
      case 5: // Changes Arpeggiator Speed
         arpeggiatorInterval = map(noteNumber,0,127,20,200);
      break;
    
      case 6: // Accepts noteNumberArray and noteDurationArray
         arpNoteNumberArray[noteNumber] = noteNumberFromArray;
         arpNoteDurationArray[noteNumber] = noteDurationFromArray;
      break;
   }
}





void dumbLeadTwoEngine(byte dataType, byte noteNumber, float floatData, 
   boolean noteComingFromArpeggiator, byte arpNoteNumberFromArray, 
   unsigned long arpNoteDurationFromArray) {

   static boolean insideRun = LOW;
   static byte noteNumberArray[8];
   static unsigned long noteDurationArray[8];
   static boolean portamentoOn = HIGH;
   static boolean portamentoRunning = LOW;
   static boolean portamentoDirection;
   unsigned long currentMicros;
   static unsigned long previousPortamentoChange;
   static unsigned long portamentoSpeedInMicros = 10000;
   static byte portamentoStepInNotes = 1;
   static int bendMod = 0;
   static byte insideNoteNumber;
   static byte insideTargetNoteNumber;
   static boolean arpeggiatorOn = LOW;
   static unsigned long decayInterval;
   static boolean decayOn = LOW;
   static byte currentDecayLevel = 0;
   static unsigned long previousDecayMicros;
   static byte engineChannel = 2;
   static byte engineChip = 0;
   static byte modwheelAmplitude = 15;
   byte insideNoteNumberAfterMods;
   static byte polynomialType = 4;
  
   switch (dataType) {
      case 1: // Main Loop Run
         if ((insideRun == HIGH) && (portamentoOn == HIGH) && 
            (portamentoRunning == HIGH) && (decayOn == LOW)) {
            
            currentMicros = micros();
            if ((portamentoDirection == LOW) && (currentMicros - 
               previousPortamentoChange >= portamentoSpeedInMicros)) {
               insideNoteNumber = insideNoteNumber - portamentoStepInNotes;
               previousPortamentoChange = micros();
               if (insideNoteNumber <= insideTargetNoteNumber) {
                  portamentoRunning = LOW;
                  insideNoteNumber = insideTargetNoteNumber;
               }
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
            }
            if ((portamentoDirection == HIGH) && (currentMicros - 
               previousPortamentoChange >= portamentoSpeedInMicros)) {
               
               insideNoteNumber = insideNoteNumber + portamentoStepInNotes;
               previousPortamentoChange = micros();
               if (insideNoteNumber >= insideTargetNoteNumber) {
                  portamentoRunning = LOW;
                  insideNoteNumber = insideTargetNoteNumber;
               }
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
            }
         }
         if ((decayOn == HIGH) && (insideRun == HIGH)) {
            currentMicros = micros();
            if ((currentMicros - previousDecayMicros) >= decayInterval) {
               writeAmplitude((modwheelAmplitude - currentDecayLevel),
                  engineChannel,engineChip);
               if (currentDecayLevel < 15) {
                  currentDecayLevel++;
               }
               previousDecayMicros = micros();
            }
         }
      break;
    
      case 2: // Note On
         if ((arpeggiatorOn == LOW) || (noteComingFromArpeggiator == HIGH)) {
            if (insideRun == LOW) {
               insideNoteNumber = noteNumber;
               insideTargetNoteNumber = noteNumber;
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               writePolynomialSelect(polynomialType,engineChannel,engineChip);
               writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
               writeAmplitude(modwheelAmplitude,engineChannel,engineChip);
               if (arpeggiatorOn == LOW) {
                  placeNoteInFirstEmptyArrayPosition(noteNumberArray,
                     noteDurationArray,noteNumber);
               }
               insideRun = HIGH;
               portamentoRunning = LOW;
            }
      
            else if ((insideRun == HIGH) && (arpeggiatorOn == LOW)) {
               insideTargetNoteNumber = noteNumber;
               portamentoRunning = HIGH;
               placeNoteInFirstEmptyArrayPosition(noteNumberArray,
                  noteDurationArray,noteNumber);
               if (insideTargetNoteNumber > insideNoteNumber) {
                  portamentoDirection = HIGH;
               }
               else {
                  portamentoDirection = LOW;
               }
            }
            currentDecayLevel = 0;
            if ((decayOn == HIGH) && (arpeggiatorOn == LOW)) {
               insideNoteNumber = insideTargetNoteNumber;
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
            }
         }
      break;
      
      case 3: // Note Off
         if ((arpeggiatorOn == LOW) || (noteComingFromArpeggiator == HIGH)) {
            if (arpeggiatorOn == LOW) {
               eraseThisNoteInArray(noteNumberArray,noteDurationArray,noteNumber);
               if (mostRecentNoteInArray(noteNumberArray,noteDurationArray) == 0) {
                  writeAmplitude(0,engineChannel,engineChip);
                  insideRun = LOW;
               }
               else if (decayOn == LOW) {
                  dumbLeadTwoEngine(2,mostRecentNoteInArray(noteNumberArray,
                     noteDurationArray),0,LOW,0,0);
               }
            }
            if ((arpeggiatorOn == HIGH) && (arpNoteNumberFromArray == 0)) {
               writeAmplitude(0,engineChannel,engineChip);
               insideRun = LOW;
            }

            // If notes are coming from main arp sequencer.
            if ((arpeggiatorOn == HIGH) && (arpNoteNumberFromArray == 1)) { 
               insideRun = LOW;
            }
         }
      break;
    
      case 4: // Pitch Bend
         if (floatData == 8192) {
            bendMod = 0;
         }
         if (floatData < 8192) {
            bendMod = map(bendRange,1,12,0,-31);
            bendMod = map(floatData,0,8191,bendMod,0);
         }
         if (floatData > 8192) {
            bendMod = map(bendRange,1,12,0,31);
            bendMod = map(floatData,8193,16383,0,bendMod); 
         }
         insideNoteNumberAfterMods = insideNoteNumber + bendMod;
         writeNote(insideNoteNumberAfterMods,engineChannel,engineChip);
      break;
    
      case 5: // ModWheel
         modwheelAmplitude = map(noteNumber,0,127,0,15);
         if ((insideRun == HIGH) && (decayOn == LOW)) {
            writeAmplitude(modwheelAmplitude,engineChannel,engineChip);
         }
      break;
    
      case 6: // CC3 Decay
         if ((noteNumber == 0) && (decayOn == HIGH)) {
            decayOn = LOW;
            portamentoOn = HIGH;
         }
         else {
            decayOn = HIGH;
            portamentoOn = LOW;
            decayInterval = map(noteNumber,0,127,100000,500);
         }
      break;
    
      case 7: // CC5 Arp Speed Change
      break;
    
      case 8: // Arpeggiator Has Turned On Or Off.  Receive or transmit array.
         if (noteComingFromArpeggiator == HIGH) {
            arpeggiatorOn = HIGH;
            for (int i=0;i<8;i++) {
               dumbLeadTwoArpeggiator(6,i,0,noteNumberArray[i],
                  noteDurationArray[i]);
            }
         }
         else {
            arpeggiatorOn = LOW;
            noteNumberArray[noteNumber] = arpNoteNumberFromArray;
            noteDurationArray[noteNumber] = arpNoteDurationFromArray;
         }
      break;
    
      case 9: // Polynomail Type Select
         polynomialType = map(noteNumber,0,127,0,15);
         writePolynomialSelect(polynomialType,engineChannel,engineChip);
      break;

      case 10: // Porta speed
         portamentoStepInNotes = map(noteNumber,0,127,1,10);
      break;
   }
}






void smartLeadTwoArpeggiator(byte dataType, byte noteNumber, byte noteVelocity, 
   byte noteNumberFromArray, unsigned long noteDurationFromArray) {
   
   static byte arpNoteNumberArray[8];
   static unsigned long arpNoteDurationArray[8];
   static boolean arpeggiatorOn;
   unsigned long currentMillis;
   static unsigned long previousMillis;
   unsigned long elapsedTimeInMillis;
   static unsigned int arpeggiatorInterval = 20;
   static byte currentArpArrayPosition;
   static boolean arpeggiatorRunning = LOW;
   byte transitionNote;

   switch (dataType) {
      case 1: // Main Loop Run
         if ((arpeggiatorOn == HIGH) && (arpeggiatorRunning == HIGH)) {
            currentMillis = millis();
            elapsedTimeInMillis = currentMillis - previousMillis;
            if (elapsedTimeInMillis >= arpeggiatorInterval) {
               smartLeadTwoEngine(3,arpNoteNumberArray[currentArpArrayPosition],
                  0,HIGH,1,0);
               if (currentArpArrayPosition < 7) {
                  currentArpArrayPosition++;
               }
               else {
                  currentArpArrayPosition = 0;
               }
               while (arpNoteNumberArray[currentArpArrayPosition] == 0) {
                  if (currentArpArrayPosition < 7) {
                     currentArpArrayPosition++;
                  }
                  else {
                     currentArpArrayPosition = 0;
                  }
               }
               smartLeadTwoEngine(2,arpNoteNumberArray[currentArpArrayPosition],
                  0,HIGH,1,0);
               previousMillis = millis();
            }
         }
      break;
      
      case 2: // Note On
         if (arpeggiatorOn == HIGH) {
            placeNoteInFirstEmptyArrayPosition(arpNoteNumberArray,
               arpNoteDurationArray,noteNumber);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
            if (arpeggiatorRunning == LOW) {
               smartLeadTwoEngine(2,noteNumber,0,HIGH,0,0);
            }
         }
      break;
    
      case 3: // Note Off
         if (arpeggiatorOn == HIGH) {
            eraseThisNoteInArray(arpNoteNumberArray,
               arpNoteDurationArray,noteNumber);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
            if (arpeggiatorRunning == LOW) {
               smartLeadTwoEngine(3,noteNumber,0,HIGH,0,0);
               
               if (testArrayContentsForNoNotes(arpNoteNumberArray) == LOW) {
                  smartLeadTwoEngine(2,findSingleNoteInArray(
                     arpNoteNumberArray),0,HIGH,0,0);
               }
            }
         }
      break;
    
      case 4: // CC4: Arpeggiator On/Off
         if ((noteNumber > 0) && (arpeggiatorOn == LOW)) {
            arpeggiatorOn = HIGH;
            smartLeadTwoEngine(8,0,0,HIGH,0,0);
            arpeggiatorRunning = testArrayContentsForAtLeastTwoNotes(
               arpNoteNumberArray) ? HIGH : LOW;
         }
         if (noteNumber == 0) {
            arpeggiatorOn = LOW;
            smartLeadTwoEngine(3,
               arpNoteNumberArray[currentArpArrayPosition],0,HIGH,0,0);
            transitionNote = mostRecentNoteInArray(
               arpNoteNumberArray,arpNoteDurationArray);
            if (transitionNote > 0) {
               smartLeadTwoEngine(2,transitionNote,0,HIGH,0,0);
            }
            for (int i=0;i<8;i++) {
               smartLeadTwoEngine(8,i,0,LOW,arpNoteNumberArray[i],
                  arpNoteDurationArray[i]);
            }
         }
      break;
    
      case 5: // Changes Arpeggiator Speed
         arpeggiatorInterval = map(noteNumber,0,127,20,200);
      break;
    
      case 6: // Accepts noteNumberArray and noteDurationArray
         arpNoteNumberArray[noteNumber] = noteNumberFromArray;
         arpNoteDurationArray[noteNumber] = noteDurationFromArray;
      break;
   }
}





void smartLeadTwoEngine(byte dataType, byte noteNumber, 
   float floatData, boolean noteComingFromArpeggiator, 
   byte arpNoteNumberFromArray, unsigned long arpNoteDurationFromArray) {

   static boolean insideRun = LOW;
   static byte noteNumberArray[8];
   static unsigned long noteDurationArray[8];
   static boolean portamentoOn = HIGH;
   static boolean portamentoRunning = LOW;
   static boolean portamentoDirection;
   unsigned long currentMicros;
   static unsigned long previousPortamentoChange;
   static unsigned long portamentoSpeedInMicros = 10000;
   static byte portamentoStepInNotes = 1;
   static int bendMod = 0;
   static byte insideNoteNumber;
   static byte insideTargetNoteNumber;
   static boolean arpeggiatorOn = LOW;
   static unsigned long decayInterval;
   static boolean decayOn = LOW;
   static byte currentDecayLevel = 0;
   static unsigned long previousDecayMicros;
   static byte engineChannel = 1;
   static byte engineChip = 0;
   static byte modwheelAmplitude = 15;
   byte insideNoteNumberAfterMods;
   static byte polynomialType = 4;

   switch (dataType) {
      case 1: // Main Loop Run
         if ((insideRun == HIGH) && (portamentoOn == HIGH) && 
            (portamentoRunning == HIGH) && (decayOn == LOW)) {

            currentMicros = micros();
            if ((portamentoDirection == LOW) && (currentMicros - 
               previousPortamentoChange >= portamentoSpeedInMicros)) {
               
               insideNoteNumber = insideNoteNumber - portamentoStepInNotes;
               previousPortamentoChange = micros();
               
               if (insideNoteNumber <= insideTargetNoteNumber) {
                  portamentoRunning = LOW;
                  insideNoteNumber = insideTargetNoteNumber;
               }
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               smartPolyWriteNote(insideNoteNumberAfterMods,
                  engineChannel,engineChip);
            }
            if ((portamentoDirection == HIGH) && (currentMicros - 
               previousPortamentoChange >= portamentoSpeedInMicros)) {
               
               insideNoteNumber = insideNoteNumber + portamentoStepInNotes;
               previousPortamentoChange = micros();
               if (insideNoteNumber >= insideTargetNoteNumber) {
                  portamentoRunning = LOW;
                  insideNoteNumber = insideTargetNoteNumber;
               }
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               smartPolyWriteNote(insideNoteNumberAfterMods,
                  engineChannel,engineChip);
            }
         }
         if ((decayOn == HIGH) && (insideRun == HIGH)) {
            currentMicros = micros();
            if ((currentMicros - previousDecayMicros) >= decayInterval) {
               writeAmplitude((modwheelAmplitude - currentDecayLevel),
                  engineChannel,engineChip);
               if (currentDecayLevel < 15) {
                  currentDecayLevel++;
               }
               previousDecayMicros = micros();
            }
         }
      break;
    
      case 2: // Note On
         if ((arpeggiatorOn == LOW) || (noteComingFromArpeggiator == HIGH)) {
            if (insideRun == LOW) {
               insideNoteNumber = noteNumber;
               insideTargetNoteNumber = noteNumber;
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               smartPolyWriteNote(insideNoteNumberAfterMods,
                  engineChannel,engineChip);
               writeAmplitude(modwheelAmplitude,engineChannel,engineChip);
               if (arpeggiatorOn == LOW) {
                  placeNoteInFirstEmptyArrayPosition(noteNumberArray,
                     noteDurationArray,noteNumber);
               }
               insideRun = HIGH;
               portamentoRunning = LOW;
            }
            else if ((insideRun == HIGH) && (arpeggiatorOn == LOW)) {
               insideTargetNoteNumber = noteNumber;
               portamentoRunning = HIGH;
               placeNoteInFirstEmptyArrayPosition(noteNumberArray,
                  noteDurationArray,noteNumber);
               if (insideTargetNoteNumber > insideNoteNumber) {
                  portamentoDirection = HIGH;
               }
               else {
                  portamentoDirection = LOW;
               }
            }
            currentDecayLevel = 0;
            if ((decayOn == HIGH) && (arpeggiatorOn == LOW)) {
               insideNoteNumber = insideTargetNoteNumber;
               insideNoteNumberAfterMods = insideNoteNumber + bendMod;
               smartPolyWriteNote(insideNoteNumberAfterMods,
                  engineChannel,engineChip);
            }
         }
      break;
      
      case 3: // Note Off
         if ((arpeggiatorOn == LOW) || (noteComingFromArpeggiator == HIGH)) {
            if (arpeggiatorOn == LOW) {
               eraseThisNoteInArray(noteNumberArray,
                  noteDurationArray,noteNumber);
               if (mostRecentNoteInArray(
                  noteNumberArray,noteDurationArray) == 0) {

                  writeAmplitude(0,engineChannel,engineChip);
                  insideRun = LOW;
               }
               else if (decayOn == LOW) {
                  smartLeadTwoEngine(2,mostRecentNoteInArray(
                     noteNumberArray,noteDurationArray),0,LOW,0,0);
               }
            }
            if ((arpeggiatorOn == HIGH) && (arpNoteNumberFromArray == 0)) {
               writeAmplitude(0,engineChannel,engineChip);
               insideRun = LOW;
            }
            
            // If notes are coming from main arp sequencer.
            if ((arpeggiatorOn == HIGH) && (arpNoteNumberFromArray == 1)) { 
               insideRun = LOW;
            }
         }
      break;
    
      case 4: // Pitch Bend
         if (floatData == 8192) {
            bendMod = 0;
         }
         if (floatData < 8192) {
            bendMod = map(floatData,0,8191,((-1) * bendRange),0);
         }
         if (floatData > 8192) {
            bendMod = map(floatData,8193,16383,0,bendRange); 
         }
         insideNoteNumberAfterMods = insideNoteNumber + bendMod;
         smartPolyWriteNote(insideNoteNumberAfterMods,
            engineChannel,engineChip);
      break;
    
      case 5: // ModWheel
         modwheelAmplitude = map(noteNumber,0,127,0,15);
         if ((insideRun == HIGH) && (decayOn == LOW)) {
            writeAmplitude(modwheelAmplitude,engineChannel,engineChip);
         }
      break;
    
      case 6: // CC3 Decay
         if ((noteNumber == 0) && (decayOn == HIGH)) {
            decayOn = LOW;
            portamentoOn = HIGH;
         }
         else {
            decayOn = HIGH;
            portamentoOn = LOW;
            decayInterval = map(noteNumber,0,127,100000,500);
         }
      break;
    
      case 7: // CC5 Arp Speed Change
      break;
    
      case 8: // Arpeggiator Has Turned On Or Off.  Receive or transmit array.
         if (noteComingFromArpeggiator == HIGH) {
            arpeggiatorOn = HIGH;
            for (int i=0;i<8;i++) {
               smartLeadTwoArpeggiator(6,i,0,noteNumberArray[i],
                  noteDurationArray[i]);
            }
         }
         else {
            arpeggiatorOn = LOW;
            noteNumberArray[noteNumber] = arpNoteNumberFromArray;
            noteDurationArray[noteNumber] = arpNoteDurationFromArray;
         }
      break;

      case 10: // Porta speed
         portamentoStepInNotes = map(noteNumber,0,127,1,10);
      break;
   }
}





//////////////////////////////////////////
//  MISC FUNCTIONS
//////////////////////////////////////////


void ledFlash(boolean dataType) {

   static unsigned long currentMillis; 
   static boolean ledOn = LOW;
   static unsigned long ledTurnOffTime; 
   static unsigned int ledOnLength = 10;

   switch (dataType) {
      case 1: // If the LED is scheduled to turn off, turn it off.
         if (ledOn == HIGH) {
            currentMillis = millis();
            if (currentMillis >= ledTurnOffTime) {
               digitalWrite(11, LOW);
            }
         }
      break;

      case 2: // Turn the LED on and set the time it is scheduled to turn off.
         digitalWrite(11,HIGH);
         ledOn = HIGH;
         currentMillis = millis();
         ledTurnOffTime = currentMillis + ledOnLength;
      break;
   }
}




//////////////////////////////////////////
//  TIA FUNCTIONS
//////////////////////////////////////////


void writeNote(byte midiNoteNumber, byte channelNumber, byte chipNumber) {
	byte tiaNote;
	
   // 1-32
   if ((midiNoteNumber > 0) && (midiNoteNumber < 33)) {  
      tiaNote = map(midiNoteNumber,1,32,31,0);
   }
   
   // 33-64
   if ((midiNoteNumber > 32) && (midiNoteNumber < 65)) {  
      tiaNote = map(midiNoteNumber,33,64,31,0);
        }
   
   // 65-96
   if ((midiNoteNumber > 64) && (midiNoteNumber < 97)) { 
      tiaNote = map(midiNoteNumber,65,96,31,0);
        }
   
   // 97-127 (only 31 notes here because 127 is the limit for 7 bit midi data)
   if ((midiNoteNumber > 96) && (midiNoteNumber < 128)) { 
      tiaNote = map(midiNoteNumber,97,127,31,1);
        }
    
   PORTD = (channelNumber + 6) & 0xF;
   PORTB = tiaNote & 0x1F;
   pulseCS(chipNumber);	
}



void smartPolyWriteNote(byte midiNoteNumber, byte channelNumber, byte chipNumber) {

   byte tiaNote[]={
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      31,
      29,
      28,
      27,
      29,
      27,
      25,
      24,
      23,
      23,
      22,
      21,
      18,
      16,
      15,
      15,
      14,
      13,
      12,
      12,
      11,
      10,
      10,
      9,
      8,
      8,

      // Pure Tones
      31,
      29,
      27,
      17,
      24,
      23,
      14,
      20,
      19,
      18,
      17,
      16,
      15,
      14,
      18,
      17,
      16,
      11,
      14,
      13,
      9,
      18,
      8,
      10,
      7,
      9,
      6,
      8,
      16,
      5,
      14,
      6,
      4,
      18,
      5,
      10,
      3,
      4,
      6,
      8,
      16,
      2,
      1,
      0 
   };

   byte polynomial[]={
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,

      // Polynomial tones, starting at position 23
      7,
      7,
      7,
      7,
      1,
      1,
      7,
      1,
      1,
      7,
      7,
      7,
      1,
      7,
      7,
      1,
      1,
      1,
      7,
      3,
      1,
      7,
      3,
      1,
      7,
      1,
      
      // Pure Tones, starting at position 49
      4,
      4,
      4,
      12,
      4,
      4,
      12,
      4,
      4,
      4,
      4,
      4,
      4,
      4,
      12,
      12,
      12,
      4,
      12,
      12,
      4,
      4,
      4,
      12,
      4,
      12,
      4,
      12,
      12,
      4,
      12,
      12,
      4,
      4,
      12,
      12,
      4,
      12,
      4,
      12,
      12,
      4,
      4,
      4 
   };

  
   if (midiNoteNumber < 23) {
      midiNoteNumber = 23;
   }

   if (midiNoteNumber > 92) {
      midiNoteNumber = 92;
   }  

  // first C# is 49, array position 49
    
   PORTD = (channelNumber + 6) & 0xF;
   PORTB = tiaNote[midiNoteNumber] & 0x1F;
   pulseCS(chipNumber);
   
   PORTD = (channelNumber + 4) & 0xF;
   PORTB = polynomial[midiNoteNumber] & 0x1F;
   pulseCS(chipNumber);
}



void writeAmplitude(byte amplitude, byte channelNumber, byte chipNumber) {
   PORTD = (channelNumber + 8) & 0xF;
   PORTB = amplitude & 0x1F;
   pulseCS(chipNumber);
}



void writePolynomialSelect(byte polynomial, 
   byte channelNumber, byte chipNumber) {

   PORTD = (channelNumber + 4) & 0xF;
   PORTB = polynomial & 0x1F;
   pulseCS(chipNumber);
}



void pulseCS(byte chipNumber) {
	
   digitalWrite(CS0pinChip0,LOW);
   delayMicroseconds(transferHoldTime);
   digitalWrite(CS0pinChip0,HIGH);
}








//////////////////////////////////////////
//  MIDI CALLBACK FUNCTIONS
//////////////////////////////////////////



void OnNoteOn (byte channel, byte note, byte velocity) {
  
   if (noVelocity == HIGH) {
      velocity = 127;
   }
  
   switch (channel) {
      case 1: //sample engine
         dumbPolyEngine(2,note,velocity);
      break;
    
      case 2:
         smartPolyEngine(2,note,velocity);
      break;
    
      case 3:
         dumbLeadOneArpeggiator(2,note,velocity,0,0);
         dumbLeadOneEngine(2,note,0,LOW,0,0);
      break;
    
      case 4:
         smartLeadOneArpeggiator(2,note,velocity,0,0);
         smartLeadOneEngine(2,note,0,LOW,0,0);
      break;
    
      case 5:
         dumbLeadTwoArpeggiator(2,note,velocity,0,0);
         dumbLeadTwoEngine(2,note,0,LOW,0,0);
      break;

      case 6:
         smartLeadTwoArpeggiator(2,note,velocity,0,0);
         smartLeadTwoEngine(2,note,0,LOW,0,0);
      break;
   }
   ledFlash(2);
}





void OnNoteOff(byte channel, byte note, byte velocity) {
  
   switch (channel) {
      case 1: //sample engine
         dumbPolyEngine(3,note,velocity);
      break;
    
      case 2:
         smartPolyEngine(3,note,velocity);
      break;
    
      case 3:
         dumbLeadOneArpeggiator(3,note,velocity,0,0);
         dumbLeadOneEngine(3,note,0,LOW,0,0);
      break;
    
      case 4:
         smartLeadOneArpeggiator(3,note,velocity,0,0);
         smartLeadOneEngine(3,note,0,LOW,0,0);
      break;
    
      case 5:
         dumbLeadTwoArpeggiator(3,note,velocity,0,0);
         dumbLeadTwoEngine(3,note,0,LOW,0,0);
      
      break;

      case 6:
         smartLeadTwoArpeggiator(3,note,velocity,0,0);
         smartLeadTwoEngine(3,note,0,LOW,0,0);
      break;
   }
}




void OnControlChange(byte channel, byte control, byte value) {

   switch(control) {
      case 1: // Mod wheel, which controls different things for different engines.
         switch (channel) {
            case 1: // mainSampleEngine(5, value, 0, 0);
               dumbPolyEngine(5,value,0);
            break;   

            case 2: // mainPolyEngine(5, value, 0);
               smartPolyEngine(5,value,0);
            break;

            case 3: // mainBassEngine(5, value, 0, LOW, 0, 0);
               dumbLeadOneEngine(5, value, 0, LOW, 0, 0);
            break;

            case 4:
               smartLeadOneEngine(5, value, 0, LOW, 0, 0);
            break;

            case 5: 
               dumbLeadTwoEngine(5, value, 0, LOW, 0, 0);
            break;

            case 6:
               smartLeadTwoEngine(5, value, 0, LOW, 0, 0);
            break;
         }
      break;
    
      case 2: // Global pitch bend range change.
         bendRange = map(value,0,127,1,12);
      break;
    
      case 3: // Decay
         switch (channel) {
            case 1:
               dumbPolyEngine(6,value,0);
            break;

            case 2: // mainPoly Decay
               smartPolyEngine(6,value,0);
            break;

            case 3: // mainBass Decay
               dumbLeadOneEngine(6,value,0,LOW,0,0);
            break;

            case 4:
               smartLeadOneEngine(6,value,0,LOW,0,0);
            break;

            case 5:
               dumbLeadTwoEngine(6,value,0,LOW,0,0);
            break;

            case 6:
               smartLeadTwoEngine(6,value,0,LOW,0,0);
            break;
         }
      break;
    
      case 4: // Arpeggiator On/Off
         switch (channel) {
            case 1: // Sample
        	
            break;

            case 2: // Poly
            break;

            case 3: // mainBass Engine
               dumbLeadOneArpeggiator(4,value,0,0,0);
            break;

            case 4: // mainNoise Engine
               smartLeadOneArpeggiator(4,value,0,0,0);
            break;

            case 5: // mainLeadOne Engine
               dumbLeadTwoArpeggiator(4,value,0,0,0);
            break;

            case 6: // mainLeadTwo Engine
               smartLeadTwoArpeggiator(4,value,0,0,0);
            break;
         }
      break;
    
      case 5: // Arpeggiator Speed
         switch (channel) {
            case 1: 
            break;
            
            case 2:
            break;
            
            case 3: // Bass Arpeggiator Speed
               dumbLeadOneArpeggiator(5,value,0,0,0);
            break;
            
            case 4:
               smartLeadOneArpeggiator(5,value,0,0,0);
            break;
            
            case 5:
               dumbLeadTwoArpeggiator(5,value,0,0,0);
            break;
            
            case 6:
               smartLeadTwoArpeggiator(5,value,0,0,0);
            break;
         }
      break;
    
      case 6: // Polynomial Type Select
         switch (channel) {
            case 1: 
               dumbPolyEngine(7,value,0);
            break;
            case 3: 
               dumbLeadOneEngine(9,value,0,LOW,0,0);
            break;
            case 5: 
               dumbLeadTwoEngine(9,value,0,LOW,0,0);
            break;
         }
      break;

      case 20: // Portamento Speed
         switch (channel) {
            case 3: 
               dumbLeadOneEngine(10,value,0,LOW,0,0);
            break;

            case 4: 
               smartLeadOneEngine(10,value,0,LOW,0,0);
            break;

            case 5: 
               dumbLeadTwoEngine(10,value,0,LOW,0,0);
            break;

            case 6: 
               smartLeadTwoEngine(10,value,0,LOW,0,0);
            break;
         }
      break;
   }
   ledFlash(2);
}




void OnPitchChange(byte channel, int pitch) {
   switch (channel) {
      case 1: // mainSampleEngine(4,0,0,pitch);
    	break;
      
      case 2: 
      break;
      
      case 3: // mainBassEngine(4, 0, bendFactor, LOW, 0, 0);
         dumbLeadOneEngine(4, 0, pitch, LOW, 0, 0);
      break;
      
      case 4:
         smartLeadOneEngine(4, 0, pitch, LOW, 0, 0); 
      break;
      
      case 5: 
         dumbLeadTwoEngine(4, 0, pitch, LOW, 0, 0);
      break;
      
      case 6:
         smartLeadTwoEngine(4, 0, pitch, LOW, 0, 0);
      break;
   }
   ledFlash(2);
}


