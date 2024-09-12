/*
 * main.h
 *
 * Created: 9/11/2024 10:47:10 AM
 *  Author: mikey
 */ 


#ifndef MAIN_H_
#define MAIN_H_


// Event handlers
void register_midi_events();
void unregister_midi_events();
void learn_channel_note_on(uint8_t midi_note);
void learn_channel_progress();
void check_calibration_mode();
void output_calibration_voltage();
void update_blink_counter();
void iterate_voct_calibration();
void check_learn_switch();


#endif /* MAIN_H_ */