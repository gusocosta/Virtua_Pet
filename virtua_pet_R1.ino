/* ==================================== Virtua Pet R1 ================================
Hi. This is a tiny little project I made to get my own Virtual Pet. I hope you like it
as much as I do.
The code can receive some optimiztion, so feel free to change it as you like.
All the bitmap images are on the bitmap.h file.
I would be glade to see your project too if you choose to make one pet by yourself.
Thank you!

HARDWARE:
- STM32 F103C8T6 (Blue Pill); (It demands at least 3.5KB RAM and 32KB ROM)
- 1.3" OLED I2C Screen 128x64;
- 4 push buttons (with 1n4148 diode to prevent debounce);

by Gus Costa
(GitHub: gusocosta)
(Reddit: gu-ocosta)
======================================================================================
*/

#include <GyverOLED.h>
#include "bitmaps.h"
GyverOLED<SSH1106_128x64> oled;

//Peripheral constants
const uint8_t bt_down = PA9;
const uint8_t bt_right = PA10;
const uint8_t bt_up = PA11;
const uint8_t bt_left = PA12;

//Time vars and const
const uint8_t check_pet_time = 5; //in sec
uint32_t draw_tick = 0;
uint8_t draw_tick_max = 100;
uint16_t act_time[4] = {0,0,0,0};
uint32_t current_millis = 0;
uint32_t previous_millis = 0;
uint32_t ani_tick = 0;
uint32_t ani_tick_previous = 0;
uint32_t ani_tick_previous_millis = 0;
uint32_t screen_tick = 0;
uint32_t screen_tick_previous = 0;
uint32_t buttom_cooldown_millis = 0;
uint8_t buttom_cooldown = 200;

uint32_t doing_tick = 0;
uint32_t doing_cooldown = 2000; 
uint32_t sleeping_time = 0;
bool ani_icon_playing = false;

//Pet vars and const
uint8_t pet_species = 0;
byte pet_sex = 0;
uint8_t pet_lvl = 0;
const uint8_t species_amount = 5;
String pet_all_species[species_amount] = {"Egg", "Slime", "Bunny", "Dino", "T-Rex"};
uint8_t pet_size[species_amount] = {24, 24, 32, 64, 64};
int pet_eating_vars[species_amount][5][5] ={ //[species][food][status change]
		//food> 0: eating tomato, 1: eating carrot, 2: eating mushroom, 3: eating chicken, 4: eating candy
		//status> 0: health, 1: hungry, 2: chean, 3: happy, 4: sleep
		{{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}}, //egg
		{{2,10,0,-5,0},{2,10,0,-5,0},{2,15,0,0,0},{2,15,0,10,0},{-10,25,0,20,0}}, //slime 
		{{2,10,0,5,0},{2,25,0,15,0},{2,15,0,0,0},{-10,5,0,-5,0},{-10,20,0,10,0}}, //bunny
		{{2,25,0,15,0},{2,10,0,0,0},{2,15,0,0,0},{-2,5,0,-5,0},{-10,20,0,10,0}}, //dino
		{{-2,5,0,-5,0},{-2,5,0,-5,0},{-2,5,0,-5,0},{20,20,0,15,0},{-10,20,0,10,0}}}; //t_rex
String pet_age = "0m";
typedef struct {
  uint8_t health;   //0-100
  uint8_t hungry;   //0-100
  uint8_t clean;    //0-100
  uint8_t happy;    //0-100
  uint16_t sleep;   //0-200
} pet_stats;
pet_stats pet = {100,80,80,80,200};
uint8_t pet_mood = 0; //0: normal, 1: happy, 2: sleep, 3:dead
uint8_t pet_doing = 0; //0: nothing, 1: eating tomato, 2: eating carrot, 3: eating mushroom, 4: eating chicken, 5: eating candy, 6: cleaning, 7: petting, 8: sleeping, 9: dead
bool pet_alive = true;
bool pet_sleeping = false;
bool pet_poop = false;
uint8_t pet_ani = 0;
uint8_t food_check[5] = {0,0,0,0,0};
bool happy_fail = false; //vars to check for evolution
bool health_fail = false; //vars to check for evolution

//UI vars and consts
uint32_t rng_comp = 0;
int rng_dir = 1;
uint8_t screen = 1; //0: name, 1: main, 2: status
uint8_t sel = 0;
uint8_t food_sel = 0; //0: tomato, 1: carrot, 2: mushroom, 3: chicken, 4: candy
const uint8_t bottom_bar_size = 5;
uint8_t bottom_bar_sel_size = 0;
uint8_t bottom_bar_pos[bottom_bar_size] ={};
uint8_t explosion_rad = 0;
bool evolve_ani = false;


//Sprite arrays for the animations
const unsigned char* bottom_bar_icons[bottom_bar_size-1] = {icon_hand, icon_clean, icon_sleep, icon_stat};
const unsigned char* bottom_bar_food[5] = {icon_tomato, icon_carrot, icon_mushroom, icon_chickenleg, icon_candy};
const unsigned char* icon_popup[9] = {icon_tomato, icon_carrot, icon_mushroom, icon_chickenleg, icon_candy, icon_clean, icon_hand, icon_sleep, icon_dead};

const unsigned char* slime_bot[1][1] = {{slime_bot1}};
const unsigned char* slime_up[1][2] = {{slime_up1, slime_up2}};
const unsigned char* slime_eyes[3][2] = {{slime_eye_n1, slime_eye_n2},{slime_eye_h1, slime_eye_h2},{slime_eye_s1, slime_eye_s2}};

const unsigned char* bunny_bot[2][2] = {{bunny_bot_n1, bunny_bot_n2},{bunny_bot_h1, bunny_bot_h2}};
const unsigned char* bunny_up[2][2] = {{bunny_up_n1, bunny_up_n2},{bunny_up_h1, bunny_up_h2}};
const unsigned char* bunny_eyes[3][2] = {{bunny_eye_n1, bunny_eye_n2},{bunny_eye_h1, bunny_eye_h2},{bunny_eye_s1, bunny_eye_s2}};

const unsigned char* dino_back[1][2] = {{dino_back1, dino_back2}};
const unsigned char* dino_bot[1][2] = {{dino_bot1, dino_bot2}};
const unsigned char* dino_up[1][2] = {{dino_up1, dino_up2}};
const unsigned char* dino_eyes[3][2] = {{dino_eye_n1, dino_eye_n2},{dino_eye_h1, dino_eye_h2},{dino_eye_s1, dino_eye_s2}};

const unsigned char* trex_back[1][2] = {{trex_back1, trex_back2}};
const unsigned char* trex_bot[1][2] = {{trex_bot1, trex_bot2}};
const unsigned char* trex_up[1][2] = {{trex_up1, trex_up2}};
const unsigned char* trex_eyes[3][2] = {{trex_eye_n1, trex_eye_n2},{trex_eye_h1, trex_eye_h2},{trex_eye_s1, trex_eye_s2}};

void setup() {
  Serial.begin(9600);
  pinMode(bt_left, INPUT_PULLUP);
  pinMode(bt_right, INPUT_PULLUP);
  pinMode(bt_up, INPUT_PULLUP);
  pinMode(bt_down, INPUT_PULLUP);
  //pinMode(buzz, OUTPUT);
	
  oled.init();
  oled.clear();
	bottom_bar_gen();
	age_gen();

	rng_comp = int(millis()+analogRead(PA0));
	if (rng_comp % 2 == 0){pet_sex = 1;}
}

// Generates the bottom bar
void bottom_bar_gen(){
	bottom_bar_sel_size = 127 / bottom_bar_size;
	for(uint8_t i = 0; i < bottom_bar_size; i++){
		bottom_bar_pos[i] = (i* bottom_bar_sel_size) + ((bottom_bar_sel_size - 8)/2);
	}
}

// Make the age string on the status screen
void age_gen(){
	if (pet_alive == true){
		if (act_time[3]!=0){
			pet_age = String(act_time[3]) + "D";
		}
		else if (act_time[2]!=0){
			pet_age = String(act_time[2]) + "H";
		}
		else if (act_time[1]!=0){
			pet_age = String(act_time[1]) + "m";
		}
		else{
			pet_age = "0m";
		}
	}
}
//========================================================================
//============================= Time Function ============================
//========================================================================
void time_check(){
  current_millis = millis();
	//Check animation tick
	if (current_millis - ani_tick_previous_millis >= 500){ //check seconds
    ani_tick_previous_millis = current_millis;
    ani_tick += 1;

		//Do Action
		if (current_millis > doing_tick){
			if(pet_doing < 8){
				pet_doing = 0;
				if(pet_mood == 1){
					pet_mood = 0;
				}
			}
		}
	}

	//Check time change
  if (current_millis - previous_millis >= 1000){ //check seconds
		rng_comp = int(analogRead(PA0)) * rng_dir;
		rng_dir *= -1;

    previous_millis = current_millis;
    act_time[0] += 1;
		if (act_time[0] == 5 && pet_lvl == 0){
			evolve_ani = true;
		}

    if (act_time[0] >= 60){ //check minutes
      act_time[0] = 0;
      act_time[1] += 1;
			if (act_time[1] % check_pet_time == 0){
				check_pet();
			}

      if (act_time[1] >= 60){ //check hours
        act_time[1] = 0;
        act_time[2] += 1;
			}

			if (act_time[2] >= 24){ //check days
				act_time[2] = 0;
				act_time[3] += 1;
				if (pet_lvl == 1){
					evolve_ani = true;
				}
			}
      
    }
  }
}

void check_bt(){
	//========================================================================
	//=============================== Button Up ==============================
	//========================================================================
	if (digitalRead(bt_up) == LOW){
		rng_comp += int(buttom_cooldown_millis - millis()) * rng_dir;
		rng_dir *= -1;

		if(millis() > buttom_cooldown_millis + buttom_cooldown){ //debounce
			buttom_cooldown_millis = millis();
			
			//Main Screen
			if (screen == 1){
        if (pet_alive == true && doing_tick < millis()){
          if (sel == 0){ //Eating
            if (pet.hungry < 100){
              pet_doing = food_sel + 1;
							//diferent stats per food

							pet.health += pet_eating_vars[pet_species][food_sel][0];   
							pet.hungry += pet_eating_vars[pet_species][food_sel][1];  
							pet.clean += pet_eating_vars[pet_species][food_sel][2];
							pet.happy += pet_eating_vars[pet_species][food_sel][3];
							pet.sleep += pet_eating_vars[pet_species][food_sel][4];

							food_check[food_sel] += 1;
              pet_mood = 1;
							pet_sleeping = false;
            }
          }

          else if (sel == 1){ //Peting
            if (pet.happy < 100){
              pet_doing = 7;
              pet.happy += 10;
              pet_mood = 1; 
							pet_sleeping = false;
            }
          }

          else if (sel == 2){ //Cleaning
            if (pet_poop == true){
              pet_poop = false;
            }
            if (pet.clean < 100){
              pet_doing = 6;
              pet.clean = 100;
              pet_mood = 1;
							pet_sleeping = false;
            }
          }
          else if (sel == 3){ //Sleeping
            if (pet_doing == 0 && pet_sleeping == false){
							pet_mood = 2;
							pet_doing = 8;
							pet_sleeping = true;
            }
            else if (pet_doing == 8){
              pet_mood = 0;
              pet_doing = 0;
							pet_sleeping = false;
            }
          }
          else if (sel == 4){ //status
            age_gen();
            screen = 2;
            sel = 0;
          }
					doing_tick = millis() + doing_cooldown;
        }
      }
			//Status Screen
			else if (screen == 2){
				screen = 1;
			}
		clamp_pet();
		}
	}

	//========================================================================
	//============================== Button Down =============================
	//========================================================================
	if (digitalRead(bt_down) == LOW){
		rng_comp += int(buttom_cooldown_millis - millis()) * rng_dir;
		rng_dir *= -1;

		if(millis() > buttom_cooldown_millis + buttom_cooldown){ //debounce
			buttom_cooldown_millis = millis();

			if (screen == 1){
        if (pet_alive == true && doing_tick < millis()){
          if (sel == 0){ //Eating
						food_sel = func_select(food_sel, 4, 1);

					}
				}
			}

		}
	}

	//========================================================================
	//============================== Button Left =============================
	//========================================================================
	if (digitalRead(bt_left) == LOW){
		rng_comp += int(buttom_cooldown_millis - millis()) * rng_dir;
		rng_dir *= -1;
		if(millis() > buttom_cooldown_millis + buttom_cooldown){ //debounce
			buttom_cooldown_millis = millis();

			//Main Screen
			if (screen == 1){
				sel = func_select(sel, bottom_bar_size-1, 0);
			}
			
			//Status Screen
			else if (screen == 2){
				screen = 1;
			}
		}
	}

	//========================================================================
	//============================= Button Right =============================
	//========================================================================
	if (digitalRead(bt_right) == LOW){
		rng_comp += int(buttom_cooldown_millis - millis()) * rng_dir;
		rng_dir *= -1;
		if(millis() > buttom_cooldown_millis + buttom_cooldown){ //debounce
			buttom_cooldown_millis = millis();

			//Main Screen
			if (screen == 1){
				sel = func_select(sel, bottom_bar_size - 1, 1);
			}

			//Status Screen
			else if (screen == 2){
			
			}
		}
	}
}

uint8_t func_select(uint8_t var, uint8_t max, uint8_t dir){
	if(dir > 0){
		if(var < max){var += 1;}
		else{var = 0;}
	}
	else{
		if(var > 0){var -= 1;}
		else{var = max;}
	}
	return var;
}

//========================================================================
//======================= Physiological Functions ========================
//========================================================================
void check_pet(){
	if (pet_alive == true){
		//awake
		if (pet_sleeping == false){
			pet.sleep -= 1;
			pet.hungry -= 2;
			pet.clean -= 1;
			if (sleeping_time >= 10){sleeping_time -= 10;}
			else{sleeping_time = 0;}

			//Random Events
			if (int(rng_comp % 3) == 0){pet.hungry -= 2;}
			if (int(rng_comp % 4) == 0){
				pet.clean -= 1;
				pet.happy -= 2;
			}
			//poop
			if (int(rng_comp % 10) == 0){
				pet_poop = true;
			}
		}
		//sleeping
		else{
			sleeping_time += 5;
			if (sleeping_time > 480){
				pet.hungry -= 2;
				pet.clean -= 2;
				pet.health -= 1;
			}
			pet.sleep += 5;
			if (int(rng_comp % 3) == 0){pet.hungry -= 1;}
		}

		if (pet_poop == true){
			pet.health -= 2;
			pet.clean -= 3;
			pet.happy -= 2;
		}


		//happiness calc
		pet.happy -= ((100 - pet.health) + (100 - pet.clean) + (200 - pet.sleep) + (100 - pet.hungry))/40;
		
		//health calc
		if((pet.happy + pet.clean + (pet.sleep/2) + pet.hungry)/4 > 80){
			pet.health += 5;
		}
		else if((pet.happy + pet.clean + (pet.sleep/2) + pet.hungry)/4 > 50){
			pet.health -= 1;
		}
		else if((pet.happy + pet.clean + (pet.sleep/2) + pet.hungry)/4 > 30){
			pet.health -= 3;
		}

		//check death
		if (pet.health <=0 || pet.hungry <=0 || pet.sleep <= 0 || pet.clean <=0){
			pet_alive= false;
			pet_doing = 9;
			pet_mood = 3;
		}
	}

	if (pet.health <= 50){
		health_fail = true;
	}
	if (pet.happy <= 70){
		happy_fail = true;
	}

	clamp_pet();
}

void evolve(){
	switch (pet_lvl){
		case 0:
			pet_species = 1;
		break;
		case 1:
			pet_species = 2; //evolve to Bunny
			if (happy_fail == false && food_check[0] == 0 && food_check[1] == 0 && food_check[2] == 0){ //evolve to T-Rex
				pet_species = 4;
			}
			else if (health_fail == false && food_check[0] > 0 && food_check[1] > 0 && food_check[2] > 0 && food_check[3] == 0){ //evolve to Dino
				pet_species = 3;
			}
		break;
	}
	pet_lvl += 1;
}

void clamp_pet(){
	if(pet.health > 100){pet.health = 100;}
	if(pet.hungry > 100){pet.hungry = 100;}
	if(pet.clean > 100){pet.clean = 100;}
	if(pet.happy > 100){pet.happy = 100;}
	if(pet.sleep > 200){pet.sleep = 200;}
}

void ani_check(){
		if (pet_mood == 0) {
				pet_ani = ani_tick % 2;
		}
		else if (pet_mood == 1) {
				pet_ani = 2 + (ani_tick % 2);
		}
		else if (pet_mood == 2) {
				pet_ani = 4 + (ani_tick % 2);
		}
		else if (pet_mood == 3) {
				pet_ani = 5;
		}
}

void resize(const uint8_t* bmp, uint16_t w, uint16_t h, int16_t x0, int16_t y0, uint8_t scale) {
  if (!scale) return; // avoid divide-by-zero

  for (uint16_t sy = 0; sy < h; sy++) {
    uint16_t pageOffset = (sy >> 3) * w;  // which 8‑pixel row page
    uint8_t  mask       = 1 << (sy & 7);  // bit inside that page

    for (uint16_t sx = 0; sx < w; sx++) {
      uint8_t b = pgm_read_byte(bmp + pageOffset + sx);
      if (b & mask) {
        int16_t dx = x0 + sx * scale;
        int16_t dy = y0 + sy * scale;
        
				// draw the scale×scale block
        for (uint8_t yy = 0; yy < scale; yy++) {
          int16_t py = dy + yy;
          if (py < 0 || py >= 64) continue;
          for (uint8_t xx = 0; xx < scale; xx++) {
            int16_t px = dx + xx;
            if (px < 0 || px >= 128) continue;
            oled.dot(px, py);
          }
        }
      }
    }
  }
}
//========================================================================
//=========================== Draw Function ==============================
//========================================================================
void draw(){
  oled.clear();
	oled.invertText(false);
  
	//print time
	oled.setCursorXY(2, 0);
	oled.print("Day:");
	oled.print(act_time[3]);

  oled.setCursorXY(96, 0);
  if (act_time[2] < 10){oled.print("0");}
  oled.print(act_time[2]);
  oled.print(":");
  if (act_time[1] < 10){oled.print("0");}
  oled.print(act_time[1]);
	
	//print R.I.P.
	if (pet_alive == false){
		oled.setCursorXY(48, 0);
		oled.print("R.I.P.");
	}

	//print upper bar
	oled.setCursorXY(0, 0);
	oled.line(0, 10, 127, 10);

	//========================================================================
	//===============================Main Screen==============================
	//========================================================================
	if (screen == 1){
		//print char
		
		ani_check();
		draw_pet();
		
		//draw icon
		if (pet_doing > 0){
			if (pet_alive == true){
				//0: nothing, 1: eating tomato, 2: eating carrot, 3: eating mushroom, 4: eating chicken, 5: eating candy, 6: petting, 7: cleaning, 8: sleeping, 9: dead
				resize(icon_popup[pet_doing-1], 8, 8, (68 + (pet_size[pet_species]/2) + (ani_tick % 2)), (16 + ((ani_tick % 2)*2)), 2); //const uint8_t* bmp, uint16_t w, uint16_t h, int16_t x0, int16_t y0, uint8_t scale
			}
			else{
				resize(icon_popup[8],8 ,8 ,68 + (pet_size[pet_species]/2) + (ani_tick % 2), 16 + ((ani_tick % 2)*2), 2);
			}
		}

		if (pet_poop == true){
			 resize(icon_poop, 8, 8, 112, 32, 2); //bmp, w, h, x0, y0, scale
		}

		//draw bottom bar
		for (uint8_t i = 0; i < bottom_bar_size; i++){
			uint8_t invert_sprite = 0;
			//print selection
			if (i == sel){
				oled.rect((i*bottom_bar_sel_size), 54, ((i+1)*bottom_bar_sel_size), 63);
				invert_sprite = 1;
			}
			if (i == 0 ){
				oled.drawBitmap(bottom_bar_pos[i], 55, bottom_bar_food[food_sel], 8, 8, 0, invert_sprite);
			}
			else{
				oled.drawBitmap(bottom_bar_pos[i], 55, bottom_bar_icons[i-1], 8, 8, 0, invert_sprite);
			}
		}
	}

	//========================================================================
	//=============================Status Screen==============================
	//========================================================================
	else if(screen == 2){
		//Draw Infos
		if (pet_sex == 0) {oled.drawBitmap(1, 16, icon_fem, 8, 8);}
		else {oled.drawBitmap(1, 16, icon_male, 8, 8);}
		oled.setCursorXY(12, 16);
		oled.print(pet_all_species[pet_species]);
		oled.setCursorXY(1, 28);
		oled.print("Age: ");
		oled.print(pet_age);
		oled.setCursorXY(1, 40);
		oled.print("Lvl: ");
		if(pet_lvl < 2){oled.print(pet_lvl);}
		else{oled.print("MAX");}

		//Draw graffics
		for (uint8_t i = 0; i < 5; i++){
			oled.rect(64 + (i*13), 16, 72 + (i*13), 54,2);
			if(i==0){
				oled.drawBitmap(64 + (i*13), 56, icon_health, 8, 8);
				oled.rect(66 + (i*13), 18 + (34 - (34 * pet.health)/100), 70 + (i*13), 52);
			}
			else if(i==1){
				oled.drawBitmap(64 + (i*13), 56, icon_sleep, 8, 8);
				oled.rect(66 + (i*13), 18 + (34 - (34 * pet.sleep)/200), 70 + (i*13), 52);
			}
			else if(i==2){
				oled.drawBitmap(64 + (i*13), 56, icon_chickenleg, 8, 8);
				oled.rect(66 + (i*13), 18 + (34 - (34 * pet.hungry)/100), 70 + (i*13), 52);
			}
			else if(i==3){
				oled.drawBitmap(64 + (i*13), 56, icon_clean, 8, 8);
				oled.rect(66 + (i*13), 18 + (34 - (34 * pet.clean)/100), 70 + (i*13), 52);
			}
			else if(i==4){
				oled.drawBitmap(64 + (i*13), 56, icon_happiness, 8, 8);
				oled.rect(66 + (i*13), 18 + (34 - (34 * pet.happy)/100), 70 + (i*13), 52);
			}
		}

		//Draw Back Button
		oled.rect(0, 55, 6, 63);
		oled.setCursor(1, 7);
		oled.invertText(true);
		oled.print("<");
		oled.invertText(false);
	}

	if (evolve_ani == true){
		draw_explosion();
	}
  oled.update();
}

//Solving the sprite puzzle
void draw_pet(){
	uint8_t animation_eyes;
	uint8_t animation_body;
	if (pet_alive == true){
		animation_eyes = pet_mood;
		animation_body = ani_tick%2;
	}
	else{
		animation_eyes = 3;
		animation_body = 0;
	}

	switch (pet_species){
		//Egg
		case 0:
			resize(egg,12,13,52,22,2); //bmp, w, h, x0, y0, scale
		break;
		//Slime
		case 1:
			resize(slime_up[0][animation_body],12,4,52,30,2); //bmp, w, h, x0, y0, scale
			resize(slime_eyes[animation_eyes][animation_body],12,3,52,38,2);
			resize(slime_bot[0][0],12,2,52,44,2);
		break;
		//Bunny
		case 2:
			if (pet_alive == true){
				if(pet_mood != 1){
					resize(bunny_up[0][animation_body],16,6,48,16,2);
					resize(bunny_bot[0][animation_body],16,7,48,34,2);
					} 
				else{
					resize(bunny_up[1][animation_body],16,6,48,16,2);
					resize(bunny_bot[1][animation_body],16,7,48,34,2);
					} 
				resize(bunny_eyes[animation_eyes][animation_body],16,3,48,28,2);
			}
		break;
		//Dino
		case 3:
			resize(dino_back[0][animation_body],16,16,32,16,2); //bmp, w, h, x0, y0, scale
			resize(dino_up[0][animation_body],16,7,64,16,2);
			resize(dino_eyes[animation_eyes][animation_body],16,3,64,30,2);
			resize(dino_bot[0][animation_body],16,6,64,36,2);
			
		break;
		//Dino
		case 4:
			resize(trex_back[0][animation_body],16,16,32,16,2); //bmp, w, h, x0, y0, scale
			resize(trex_up[0][animation_body],16,4,64,16,2);
			resize(trex_eyes[animation_eyes][animation_body],16,3,64,24,2);
			resize(trex_bot[0][animation_body],16,9,64,30,2);
		break;
		
	}
}

//Explosion animation and evolve call
void draw_explosion(){
	explosion_rad += 3;
	if (explosion_rad < 85){
		draw_tick_max = 0;
		oled.circle(64, 32, explosion_rad + 10, 2);
		oled.circle(64, 32, explosion_rad + 3, 2);
		oled.circle(64, 32, explosion_rad, 1);
		oled.circle(64, 32, explosion_rad -10, 0);
	}
	else{
		draw_tick_max = 100;
		evolve_ani = false;
		evolve();
		explosion_rad = 0;
	}
}

void loop() {
  check_bt();
  time_check();

	if(millis() > draw_tick + draw_tick_max){ //controls the screens refresh to save some battery
		draw_tick = millis();
		draw();
	}
}