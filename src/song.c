#include <allegro.h>
#include <errno.h>
#include <time.h>
#include "main.h"
#include "editor.h"
#include "beat.h"
#include "song.h"
#include "legacy.h"
#include "midi.h"	//For eof_get_ts()
#include "midi_data_import.h"
#include "mix.h"
#include "rs.h"			//For eof_pro_guitar_track_find_effective_fret_hand_position_definition()
#include "silence.h"	//For save_wav()
#include "tuning.h"
#include "undo.h"
#include "utility.h"
#include "menu/beat.h"	//For eof_menu_beat_delete_logic()
#include "menu/edit.h"
#include "menu/file.h"
#include "menu/note.h"	//For eof_feedback_mode_update_note_selection()
#include "menu/song.h"
#include "menu/track.h"	//For the tech view enable/disable functions
#include "agup/agup.h"

#ifdef USEMEMWATCH
#include "memwatch.h"
#endif

EOF_TRACK_ENTRY eof_default_tracks[EOF_TRACKS_MAX + 1] =
{
	{0, 0, 0, 0, "", "", 0xFF, 0, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_GUITAR, 0, "PART GUITAR", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_BASS, 0, "PART BASS", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_GUITAR_COOP, 0, "PART GUITAR COOP", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_RHYTHM, 0, "PART RHYTHM", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM, 0, "PART DRUMS", "", 0xFF, 5, 0xFF000000},
	{EOF_VOCAL_TRACK_FORMAT, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "PART VOCALS", "", 0xFF, 5, 0xFF000000},
	{EOF_LEGACY_TRACK_FORMAT, EOF_KEYS_TRACK_BEHAVIOR, EOF_TRACK_KEYS, 0, "PART KEYS", "", 0xFF, 5, 0},
	{EOF_PRO_GUITAR_TRACK_FORMAT, EOF_PRO_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_PRO_BASS, 0, "PART REAL_BASS", "", 0xFF, 5, 0},
	{EOF_PRO_GUITAR_TRACK_FORMAT, EOF_PRO_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_PRO_GUITAR, 0, "PART REAL_GUITAR", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DANCE_TRACK_BEHAVIOR, EOF_TRACK_DANCE, 0, "PART DANCE", "", 0xFF, 5, 0},
	{EOF_PRO_GUITAR_TRACK_FORMAT, EOF_PRO_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_PRO_BASS_22, 0, "PART REAL_BASS_22", "", 0xFF, 5, 0},
	{EOF_PRO_GUITAR_TRACK_FORMAT, EOF_PRO_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_PRO_GUITAR_22, 0, "PART REAL_GUITAR_22", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM_PS, 0, "PART REAL_DRUMS_PS", "", 0xFF, 5, 0},

	//This pro format is not supported yet, but the entry describes the track's details
	{EOF_PRO_KEYS_TRACK_FORMAT, EOF_PRO_KEYS_TRACK_BEHAVIOR, EOF_TRACK_PRO_KEYS, 0, "PART REAL_KEYS", "", 0xFF, 5, 0}
};	//These entries describe the tracks that EOF should present by default
	//These entries are indexed the same as the track type in the new EOF project format

EOF_TRACK_ENTRY eof_midi_tracks[EOF_TRACKS_MAX + 12] =
{
	{0, 0, 0, 0, "", "", 0, 0, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_GUITAR, 0, "PART GUITAR", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_BASS, 0, "PART BASS", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_GUITAR_COOP, 0, "PART GUITAR COOP", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_RHYTHM, 0, "PART RHYTHM", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM, 0, "PART DRUMS", "", 0xFF, 5, 0},
	{EOF_VOCAL_TRACK_FORMAT, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "PART VOCALS", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_KEYS_TRACK_BEHAVIOR, EOF_TRACK_KEYS, 0, "PART KEYS", "", 0xFF, 5, 0},
	{EOF_PRO_GUITAR_TRACK_FORMAT, EOF_PRO_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_PRO_BASS, 0, "PART REAL_BASS", "", 0xFF, 5, 0},
	{EOF_PRO_GUITAR_TRACK_FORMAT, EOF_PRO_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_PRO_GUITAR, 0, "PART REAL_GUITAR", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DANCE_TRACK_BEHAVIOR, EOF_TRACK_DANCE, 0, "PART DANCE", "", 0xFF, 5, 0},
	{EOF_PRO_GUITAR_TRACK_FORMAT, EOF_PRO_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_PRO_BASS_22, 0, "PART REAL_BASS_22", "", 0xFF, 5, 0},
	{EOF_PRO_GUITAR_TRACK_FORMAT, EOF_PRO_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_PRO_GUITAR_22, 0, "PART REAL_GUITAR_22", "", 0xFF, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM_PS, 0, "PART REAL_DRUMS_PS", "", 0xFF, 5, 0},

	//These tracks are not supported for import yet, but these entries describe the tracks' details
	{EOF_PRO_KEYS_TRACK_FORMAT, EOF_PRO_KEYS_TRACK_BEHAVIOR, EOF_TRACK_PRO_KEYS, 0, "PART REAL_KEYS_X", "", EOF_NOTE_AMAZING, 5, 0},
	{EOF_PRO_KEYS_TRACK_FORMAT, EOF_PRO_KEYS_TRACK_BEHAVIOR, EOF_TRACK_PRO_KEYS, 0, "PART REAL_KEYS_H", "", EOF_NOTE_MEDIUM, 5, 0},
	{EOF_PRO_KEYS_TRACK_FORMAT, EOF_PRO_KEYS_TRACK_BEHAVIOR, EOF_TRACK_PRO_KEYS, 0, "PART REAL_KEYS_M", "", EOF_NOTE_EASY, 5, 0},
	{EOF_PRO_KEYS_TRACK_FORMAT, EOF_PRO_KEYS_TRACK_BEHAVIOR, EOF_TRACK_PRO_KEYS, 0, "PART REAL_KEYS_E", "", EOF_NOTE_SUPAEASY, 5, 0},
	{EOF_VOCAL_TRACK_FORMAT, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "HARM1", "", 0xFF, 5, 0},
	{EOF_VOCAL_TRACK_FORMAT, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "HARM2", "", 0xFF, 5, 0},
	{EOF_VOCAL_TRACK_FORMAT, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "HARM3", "", 0xFF, 5, 0},
	{EOF_VOCAL_TRACK_FORMAT, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "PART HARM1", "", 0xFF, 5, 0},
	{EOF_VOCAL_TRACK_FORMAT, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "PART HARM2", "", 0xFF, 5, 0},
	{EOF_VOCAL_TRACK_FORMAT, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "PART HARM3", "", 0xFF, 5, 0}
};	//These entries describe tracks in the RB3 MIDI file format and their associated EOF track type macros, for use during import/export
	//One complication here is that the pro keys charting gets one MIDI track for each difficulty
	//Another is that the pro guitar could have a separate track for use with the Squier guitar (22 frets instead of 17)
	//Another is that although the best way to offer harmony charting would be in PART VOCALS, each harmony gets its own MIDI track
	//For the purpose of MIDI import, it will be expected that eof_midi_tracks[track_type].track_format is correct for the given track,
	// so eof_midi_tracks[EOF_TRACK_PRO_GUITAR].track_format references the correct behavior for the track
	//The pro keys track definitions specify a difficulty level because notes in each of those tracks all belong to specific difficulties

EOF_TRACK_ENTRY eof_power_gig_tracks[EOF_POWER_GIG_TRACKS_MAX] =
{
	{0, 0, 0, 0, "", "", 0xFF, 0, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_GUITAR, 0, "guitar_1_expert", "", EOF_NOTE_CHALLENGE, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_GUITAR, 0, "guitar_1_hard", "", EOF_NOTE_AMAZING, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_GUITAR, 0, "guitar_1_medium", "", EOF_NOTE_MEDIUM, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_GUITAR, 0, "guitar_1_easy", "", EOF_NOTE_EASY, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_GUITAR_TRACK_BEHAVIOR, EOF_TRACK_GUITAR, 0, "guitar_1_beginner", "", EOF_NOTE_SUPAEASY, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM, 0, "drums_1_expert", "", EOF_NOTE_CHALLENGE, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM, 0, "drums_1_hard", "", EOF_NOTE_AMAZING, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM, 0, "drums_1_medium", "", EOF_NOTE_MEDIUM, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM, 0, "drums_1_easy", "", EOF_NOTE_EASY, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM, 0, "drums_1_beginner", "", EOF_NOTE_SUPAEASY, 5, 0},
	{EOF_VOCAL_TRACK_FORMAT, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "vocals_1_expert", "", EOF_NOTE_SUPAEASY, 5, 0},
	{0, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "vocals_1_hard", "", 0, 5, 0},
	{0, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "vocals_1_medium", "", 0, 5, 0},
	{0, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "vocals_1_easy", "", 0, 5, 0},
	{0, EOF_VOCAL_TRACK_BEHAVIOR, EOF_TRACK_VOCALS, 0, "vocals_1_beginner", "", 0, 5, 0}
};	//These entries describe tracks in the Power Gig MIDI file format
	//The definition of a difficulty number for these legacy and vocal track definitions will allow the MIDI import
	// logic to identify that it is a Power Gig MIDI instead of a FoF/RB MIDI
	//The track format values of 0 for the lower difficulty vocal tracks indicate the track will be skipped during import

EOF_TRACK_ENTRY eof_guitar_hero_animation_tracks[EOF_GUITAR_HERO_ANIMATION_TRACKS_MAX] =
{
	{0, 0, 0, 0, "", "", 0xFF, 0, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM, 0, "TRIGGERS", "", EOF_NOTE_AMAZING, 5, 0},
	{EOF_LEGACY_TRACK_FORMAT, EOF_DRUM_TRACK_BEHAVIOR, EOF_TRACK_DRUM, 0, "BAND DRUMS", "", EOF_NOTE_AMAZING, 5, 0}
};	//These entries describe the two Guitar Hero MIDI tracks that store drum animations, used to create a drum track

/* sort all notes according to position */
int eof_song_qsort_legacy_notes(const void * e1, const void * e2)
{
	EOF_NOTE ** thing1 = (EOF_NOTE **)e1;
	EOF_NOTE ** thing2 = (EOF_NOTE **)e2;

//	allegro_message("%d\n%d", (*thing1)->pos, (*thing2)->pos);	//Debug
	//Sort first by chronological order
	if((*thing1)->pos < (*thing2)->pos)
	{
		return -1;
	}
	if((*thing1)->pos > (*thing2)->pos)
	{
		return 1;
	}
	//Sort second by difficulty
	if((*thing1)->type < (*thing2)->type)
	{
		return -1;
	}
	if((*thing1)->type > (*thing2)->type)
	{
		return 1;
	}
	//Sort third by note mask
	if((*thing1)->note < (*thing2)->note)
	{
		return -1;
	}
	if((*thing1)->note > (*thing2)->note)
	{
		return 1;
	}

	// they are equal...
	return 0;
}

int eof_song_qsort_lyrics(const void * e1, const void * e2)
{
	EOF_LYRIC ** thing1 = (EOF_LYRIC **)e1;
	EOF_LYRIC ** thing2 = (EOF_LYRIC **)e2;

	if((*thing1)->pos < (*thing2)->pos)
	{
		return -1;
	}
	if((*thing1)->pos > (*thing2)->pos)
	{
		return 1;
	}

	// they are equal...
	return 0;
}

EOF_SONG * eof_create_song(void)
{
	EOF_SONG * sp;
	unsigned long i;

 	eof_log("eof_create_song() entered", 1);

	sp = malloc(sizeof(EOF_SONG));
	if(!sp)
	{
		return NULL;
	}
	sp->tags = malloc(sizeof(EOF_SONG_TAGS));
	if(!sp->tags)
	{
		free(sp);
		return NULL;
	}
	memset(sp->tags, 0, sizeof(EOF_SONG_TAGS));	//Fill with 0s to satisfy Splint
	(void) ustrcpy(sp->tags->artist, "");
	(void) ustrcpy(sp->tags->title, "");
	(void) ustrcpy(sp->tags->frettist, "");
	(void) ustrcpy(sp->tags->year, "");
	(void) ustrcpy(sp->tags->loading_text, "");
	(void) ustrcpy(sp->tags->album, "");
	sp->tags->lyrics = 0;
	sp->tags->eighth_note_hopo = 0;
	sp->tags->eof_fret_hand_pos_1_pg = 0;
	sp->tags->eof_fret_hand_pos_1_pb = 0;
	sp->tags->tempo_map_locked = 0;
	sp->tags->highlight_unsnapped_notes = 0;
	sp->tags->accurate_ts = 1;
	sp->tags->highlight_arpeggios = 0;
	sp->tags->click_drag_disabled = 0;
	sp->tags->rs_chord_technique_export = 0;
	sp->tags->double_bass_drum_disabled = 0;
	sp->tags->unshare_drum_phrasing = 0;
	sp->tags->ini_settings = 0;
	sp->tags->ogg[0].midi_offset = 0;
	sp->tags->ogg[0].modified = 0;
	(void) ustrcpy(sp->tags->ogg[0].filename, "guitar.ogg");
	sp->tags->oggs = 1;
	sp->tags->revision = 0;
	sp->tags->difficulty = 0xFF;
	sp->resolution = 0;
	sp->tracks = 0;
	sp->legacy_tracks = 0;
	sp->vocal_tracks = 0;
	sp->pro_guitar_tracks = 0;
	sp->beats = 0;
	sp->text_events = 0;
	sp->catalog = malloc(sizeof(EOF_CATALOG));
	if(!sp->catalog)
	{
		free(sp->tags);
		free(sp);
		return NULL;
	}
	sp->catalog->entries = 0;
	for(i = 0; i < EOF_MAX_BOOKMARK_ENTRIES; i++)
	{
		sp->bookmark_pos[i] = 0;
	}
	sp->midi_data_head = sp->midi_data_tail = NULL;
	sp->fpbeattimes = 0;
	return sp;
}

void eof_destroy_song(EOF_SONG * sp)
{
	unsigned long ctr;

 	eof_log("\tClosing project", 1);
 	eof_log("eof_destroy_song() entered", 1);

	if(sp == NULL)
		return;

// 	eof_log_level &= ~2;	//Disable verbose logging
	for(ctr = sp->tracks - 1; ctr > 0; ctr--)
	{	//For each entry in the track array, empty and then free it
		eof_song_empty_track(sp, ctr);
		(void) eof_song_delete_track(sp, ctr);
	}

	for(ctr=0; ctr < sp->beats; ctr++)
	{	//For each entry in the beat array, free it
		free(sp->beat[ctr]);
	}

	for(ctr=0; ctr < sp->text_events; ctr++)
	{	//For each entry in the text event array, free it
		free(sp->text_event[ctr]);
	}

	eof_MIDI_empty_track_list(sp->midi_data_head);

	free(sp->tags);
	free(sp->catalog);

	eof_log("\tProject closed", 1);
	if(eof_recovery && (sp == eof_song))
	{	//If this EOF instance is maintaining an auto-recovery file, and the currently-open song is being destroyed
		(void) delete_file("eof.recover");	//Delete it when the active project is cleanly closed
	}
	free(sp);
	eof_silence_loaded = 0;	//When the chart is destroyed, reset this condition so that if another project is being loaded, its playback will be allowed to work if audio is present
//	eof_log_level |= 2;	//Enable verbose logging
}

EOF_SONG * eof_load_song(const char * fn)
{
	PACKFILE * fp = NULL;
	EOF_SONG * sp = NULL;
	char header[16] = {'E', 'O', 'F', 'S', 'O', 'N', 'H', 0};	//This header represents the current project format
	char rheader[16] = {0};
	short i;

 	eof_log("\tLoading project", 1);
 	eof_log("eof_load_song() entered", 1);

	if(!fn)
	{
		return 0;
	}
	fp = pack_fopen(fn, "r");
	if(!fp)
	{
		(void) snprintf(eof_log_string, sizeof(eof_log_string) - 1, "\tError loading:  Cannot open input .eof file:  \"%s\"", strerror(errno));	//Get the Operating System's reason for the failure
		eof_log(eof_log_string, 1);
		return 0;
	}
//	eof_log_level &= ~2;	//Disable verbose logging
	(void) pack_fread(rheader, 16, fp);
	if(!ustricmp(rheader, header))
	{	//Current project format
		sp = eof_create_song();
		if(!sp)
		{
			(void) pack_fclose(fp);
			return NULL;
		}
		sp->tags->accurate_ts = 0;	//For existing projects, this setting must be manually enabled in order to prevent unwanted alteration to beat timings
		if(!eof_load_song_pf(sp, fp))
		{
			(void) pack_fclose(fp);
			eof_destroy_song(sp);	//Destroy the song and return on error
			return NULL;
		}
		if(EOF_TRACK_DANCE >= sp->tracks)
		{	//If the chart loaded does not contain a dance track (a 1.8beta revision chart)
			if(eof_song_add_track(sp,&eof_default_tracks[EOF_TRACK_DANCE]) == 0)	//Add a blank dance track
			{	//If the track failed to be added
				eof_destroy_song(sp);	//Destroy the song and return on error
				(void) pack_fclose(fp);
				return NULL;
			}
		}
		if(EOF_TRACK_PRO_BASS_22 >= sp->tracks)
		{	//If the chart loaded does not contain a 22 fret pro bass track (a 1.8beta revision chart)
			if(eof_song_add_track(sp,&eof_default_tracks[EOF_TRACK_PRO_BASS_22]) == 0)	//Add a blank 22 fret pro bass track
			{	//If the track failed to be added
				eof_destroy_song(sp);	//Destroy the song and return on error
				(void) pack_fclose(fp);
				return NULL;
			}
		}
		if(EOF_TRACK_PRO_GUITAR_22 >= sp->tracks)
		{	//If the chart loaded does not contain a 22 fret pro guitar track (a 1.8beta revision chart)
			if(eof_song_add_track(sp,&eof_default_tracks[EOF_TRACK_PRO_GUITAR_22]) == 0)	//Add a blank 22 fret pro guitar track
			{	//If the track failed to be added
				eof_destroy_song(sp);	//Destroy the song and return on error
				(void) pack_fclose(fp);
				return NULL;
			}
		}
		if(EOF_TRACK_DRUM_PS >= sp->tracks)
		{	//If the chart loaded does not contain a Phase Shift drum track (a 1.8beta revision chart)
			if(eof_song_add_track(sp,&eof_default_tracks[EOF_TRACK_DRUM_PS]) == 0)	//Add a blank Phase Shift drum track
			{	//If the track failed to be added
				eof_destroy_song(sp);	//Destroy the song and return on error
				(void) pack_fclose(fp);
				return NULL;
			}
		}
	}
	else
	{	//Older project format
		sp = eof_load_notes_legacy(fp, rheader[6]);
	}
	(void) pack_fclose(fp);

	/* select correct OGG */
	if(sp)
	{
		for(i = 0; i < sp->tags->oggs; i++)
		{
			if(sp->tags->ogg[i].midi_offset == sp->beat[0]->pos)
			{
				eof_selected_ogg = i;
				break;
			}
		}
	}

//Update path variables
	(void) ustrcpy(eof_filename, fn);
	(void) replace_filename(eof_song_path, fn, "", 1024);	//Set the project folder path
	(void) replace_filename(eof_last_eof_path, eof_filename, "", 1024);
	(void) ustrcpy(eof_loaded_song_name, get_filename(eof_filename));

	eof_log("\tProject loaded", 1);
//	eof_log_level |= 2;	//Enable verbose logging
	return sp;
}

EOF_NOTE * eof_legacy_track_add_note(EOF_LEGACY_TRACK * tp)
{
	if(tp && (tp->notes < EOF_MAX_NOTES))
	{
		tp->note[tp->notes] = malloc(sizeof(EOF_NOTE));
		if(tp->note[tp->notes])
		{
			memset(tp->note[tp->notes], 0, sizeof(EOF_NOTE));
			tp->notes++;
			return tp->note[tp->notes - 1];
		}
	}
	return NULL;
}

void eof_legacy_track_delete_note(EOF_LEGACY_TRACK * tp, unsigned long note)
{
	unsigned long i;

	if(tp && (note < tp->notes))
	{
		free(tp->note[note]);
		for(i = note; i < tp->notes - 1; i++)
		{
			tp->note[i] = tp->note[i + 1];
		}
		tp->notes--;
	}
}

void eof_legacy_track_sort_notes(EOF_LEGACY_TRACK * tp)
{
	if(tp)
	{
		qsort(tp->note, (size_t)tp->notes, sizeof(EOF_NOTE *), eof_song_qsort_legacy_notes);
	}
}

long eof_fixup_previous_legacy_note(EOF_LEGACY_TRACK * tp, unsigned long note)
{
	long i;

	if(!tp || (note >= tp->notes))
		return -1;	//Invalid parameters

	if(tp)
	{
		for(i = note; i > 0; i--)
		{
			if(tp->note[i - 1]->type == tp->note[note]->type)
			{
				return i - 1;
			}
		}
	}
	return -1;
}

long eof_fixup_next_legacy_note(EOF_LEGACY_TRACK * tp, unsigned long note)
{
	long i;

	if(!tp || (note >= tp->notes))
		return -1;	//Invalid parameters

	if(tp)
	{
		for(i = note + 1; i < tp->notes; i++)
		{
			if(tp->note[i]->type == tp->note[note]->type)
			{
				return i;
			}
		}
	}
	return -1;
}

void eof_legacy_track_fixup_notes(EOF_SONG *sp, unsigned long track, int sel)
{
	unsigned long i;
	long next;
	unsigned long maxbitmask,maxlane;	//Used to find the highest usable bitmask for the track (based on numlanes).  The drum and bass tracks will be allowed to keep lane 6 automatically
	unsigned long tracknum;
	EOF_LEGACY_TRACK * tp;

	if(!sp || (track >= sp->tracks))
	{
		return;
	}
	tracknum = sp->track[track]->tracknum;
	tp = sp->legacy_track[tracknum];
	if(!sel)
	{
		if(eof_selection.current < tp->notes)
		{
			eof_selection.multi[eof_selection.current] = 0;
		}
		eof_selection.current = EOF_MAX_NOTES - 1;
	}
	maxlane = tp->numlanes;
	if((maxlane < 6) && ((tp->parent->track_behavior == EOF_DRUM_TRACK_BEHAVIOR) || (tp->parent->track_type == EOF_TRACK_BASS)))
	{	//If this is a drum track or the bass track, ensure that at least 6 lanes are allowed to be kept
		maxlane = 6;
	}
	maxbitmask = (1 << maxlane) - 1;
	for(i = tp->notes; i > 0; i--)
	{	//For each note (in reverse order)
		/* fix selections */
		if((tp->note[i-1]->type == eof_note_type) && (tp->note[i-1]->pos == eof_selection.current_pos))
		{
			eof_selection.current = i-1;
		}
		if((tp->note[i-1]->type == eof_note_type) && (tp->note[i-1]->pos == eof_selection.last_pos))
		{
			eof_selection.last = i-1;
		}

		if(tp->note[i-1]->note > maxbitmask)
		{	//If this note uses lanes that are higher than it can use
			tp->note[i-1]->note &= maxbitmask;	//Clear the invalid lanes
		}

		if(tp->parent->track_behavior == EOF_DRUM_TRACK_BEHAVIOR)
		{	//If this is a drum track
			if(((tp->note[i-1]->note & 1) == 0) || (tp->note[i-1]->type != 3))
			{	//If this note does not have a bass drum note or isn't in expert difficulty
				tp->note[i-1]->flags &= ~EOF_DRUM_NOTE_FLAG_DBASS;	//Clear the double bass flag
			}
			if((tp->note[i-1]->note & 2) == 0)
			{	//If this note does not have a red note (snare), clear flags that are specific to that lane
				tp->note[i-1]->flags &= ~EOF_DRUM_NOTE_FLAG_R_RIMSHOT;
			}
			if(((tp->note[i-1]->note & 4) == 0) || ((tp->note[i-1]->flags & EOF_DRUM_NOTE_FLAG_Y_CYMBAL) == 0))
			{	//If this note does not have a yellow cymbal, clear flags that are specific to yellow cymbals
				if((tp->note[i-1]->note & 2) == 0)
				{	//If this note also doesn't have a red note (allowing for hi hat notation during disco flip)
					tp->note[i-1]->flags &= ~EOF_DRUM_NOTE_FLAG_Y_HI_HAT_OPEN;
					tp->note[i-1]->flags &= ~EOF_DRUM_NOTE_FLAG_Y_HI_HAT_PEDAL;
					tp->note[i-1]->flags &= ~EOF_DRUM_NOTE_FLAG_Y_SIZZLE;
					tp->note[i-1]->flags &= ~EOF_DRUM_NOTE_FLAG_Y_COMBO;
				}
			}
		}

		if(eof_min_note_length && (tp->parent->track_behavior == EOF_GUITAR_TRACK_BEHAVIOR))
		{	//If the user has specified a minimum length for 5 lane guitar notes, and this is a 5 lane guitar track
			if(tp->note[i-1]->length < eof_min_note_length)
			{	//If this note's length is shorter than the minimum length
				tp->note[i-1]->length = eof_min_note_length;	//Alter the length
			}
		}

		/* delete certain notes */
		if((tp->note[i-1]->note == 0) || (tp->note[i-1]->type > 4) || (tp->note[i-1]->pos < sp->tags->ogg[eof_selected_ogg].midi_offset) || (tp->note[i-1]->pos >= eof_chart_length))
		{	//Delete the note if all lanes are clear, if it is an invalid type, if the position is before the first beat marker or if it is after the last beat marker
			eof_legacy_track_delete_note(tp, i-1);
		}

		else
		{	//The note has valid gems, type and position
			/* make sure there are no 0-length notes */
			if(tp->note[i-1]->length <= 0)
			{
				tp->note[i-1]->length = 1;
			}

			/* make sure note doesn't extend past end of song */
			if(tp->note[i-1]->pos + tp->note[i-1]->length >= eof_chart_length)
			{
				tp->note[i-1]->length = eof_chart_length - tp->note[i-1]->pos;
			}

			/* compare this note to the next one of the same type
			   to make sure they don't overlap */
			next = eof_fixup_next_legacy_note(tp, i-1);
			if(next >= 0)
			{	//If there is a note after this note
				if(tp->note[i-1]->pos == tp->note[next]->pos)
				{	//And it is at the same position as this note, merge them both
					unsigned long flags = eof_prepare_note_flag_merge(tp->note[i-1]->flags, tp->parent->track_behavior, tp->note[next]->note);	//Get a flag bitmask where all lane specific flags for lanes that the next (merging) note uses have been cleared
					tp->note[i-1]->note |= tp->note[next]->note;	//Merge the note bitmasks
					tp->note[i-1]->flags = flags | tp->note[next]->flags;	//Merge the flags
					eof_legacy_track_delete_note(tp, next);			//Delete the next note, as it has been merged with this note
				}
				else
				{	//Otherwise ensure on doesn't overlap the other improperly
					unsigned long maxlength = eof_get_note_max_length(sp, track, i - 1, 1);	//Determine the maximum length for this note, taking its crazy status into account
					if(maxlength && (eof_get_note_length(sp, track, i - 1) > maxlength))
					{	//If the note is longer than its maximum length
						eof_set_note_length(sp, track, i - 1, maxlength);	//Truncate it to its valid maximum length
					}
				}
			}
		}//The note has valid gems, type and position

		tp->note[i-1]->accent &= tp->note[i-1]->note;	//Clear all accent bits that don't have a corresponding gem that exists
	}//For each note (in reverse order)

	//Run another pass to check crazy notes overlapping with gems on their same lanes more than 1 note ahead
	for(i = 0; i < tp->notes; i++)
	{	//For each note
		if(tp->note[i]->flags & EOF_NOTE_FLAG_CRAZY)
		{	//If this note is crazy, find the next gem that occupies any of the same lanes
			next = i;
			while(1)
			{
				next = eof_fixup_next_legacy_note(tp, next);
				if(next >= 0)
				{	//If there's a note in this difficulty after this note
					if(tp->note[i]->note & tp->note[next]->note)
					{	//And it uses at least one of the same lanes as the crazy note being checked
						if(tp->note[i]->pos + tp->note[i]->length >= tp->note[next]->pos - 1)
						{	//If it does not end at least 1ms before the next note starts
							tp->note[i]->length = tp->note[next]->pos - tp->note[i]->pos - 1;	//Truncate the crazy note so it does not overlap the next gem on its lane(s)
						}
						break;
					}
				}
				else
					break;
			}
		}
	}

	if(eof_open_strum_enabled(track))
	{	//If open strumming is enabled, check to ensure that open bass doesn't conflict with other notes/statuses
		for(i = 0; i < tp->notes; i++)
		{	//For each note in the track
			if(tp->note[i]->note & 32)
			{	//If this note contains open bass (lane 6)
				tp->note[i]->note = 32;							//Clear all lanes except lane 6
				tp->note[i]->flags &= (~EOF_NOTE_FLAG_CRAZY);	//Clear the crazy status
			}
		}
	}
	if(!sel)
	{
		if(eof_selection.current < tp->notes)
		{
			eof_selection.multi[eof_selection.current] = 1;
		}
	}

//Cleanup for pro drum and accent notations
	if(sp && (tp->parent->track_behavior == EOF_DRUM_TRACK_BEHAVIOR))
	{	//If the track being cleaned is a drum track
		unsigned lastcheckedgreenpos = 0;	//This will be used to prevent cymbal cleanup from operating on the same notes multiple times
		unsigned lastcheckedbluepos = 0;
		unsigned lastcheckedyellowpos = 0;
		for(i = 0; i < tp->notes; i++)
		{	//For each note in the drum track
			if((tp->note[i]->note & 16) && (!lastcheckedgreenpos || (tp->note[i]->pos != lastcheckedgreenpos)))
			{	//If this note contains a lane 5 gem, perform green cymbal cleanup
				if(eof_check_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_G_CYMBAL))
				{	//If any notes at this position are marked as a green cymbal
					eof_set_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_G_CYMBAL,1,1);	//Mark all notes at this position as green cymbal
				}
				else
				{
					eof_set_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_G_CYMBAL,0,1);	//Mark all notes at this position as green tom
					eof_set_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_G_COMBO,0,1);	//Remove green cymbal/tom combo status
				}
				lastcheckedgreenpos = tp->note[i]->pos;	//Remember that green notes at this position were already checked and fixed if applicable
			}
			if((tp->note[i]->note & 8) && (!lastcheckedbluepos || (tp->note[i]->pos != lastcheckedbluepos)))
			{	//If this note contains a lane 4 gem, perform blue cymbal cleanup
				if(eof_check_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_B_CYMBAL))
				{	//If any notes at this position are marked as a blue cymbal
					eof_set_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_B_CYMBAL,1,1);	//Mark all notes at this position as blue cymbal
				}
				else
				{
					eof_set_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_B_CYMBAL,0,1);	//Mark all notes at this position as blue tom
					eof_set_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_B_COMBO,0,1);	//Remove blue cymbal/tom combo status
				}
				lastcheckedbluepos = tp->note[i]->pos;	//Remember that blue notes at this position were already checked and fixed if applicable
			}
			if((tp->note[i]->note & 4) && (!lastcheckedyellowpos || (tp->note[i]->pos != lastcheckedyellowpos)))
			{	//If this note contains a lane 3 gem, perform yellow cymbal cleanup
				if(eof_check_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_Y_CYMBAL))
				{	//If any notes at this position are marked as a yellow cymbal
					eof_set_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_Y_CYMBAL,1,1);	//Mark all notes at this position as yellow cymbal
				}
				else
				{
					eof_set_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_Y_CYMBAL,0,1);	//Mark all notes at this position as yellow tom
					eof_set_flags_at_legacy_note_pos(tp,i,EOF_DRUM_NOTE_FLAG_Y_COMBO,0,1);	//Remove yellow cymbal/tom combo status
				}
				lastcheckedyellowpos = tp->note[i]->pos;	//Remember that yellow notes at this position were already checked and fixed if applicable
			}
			if(track != EOF_TRACK_DRUM_PS)
			{	//If this isn't the phase shift drum track
				tp->note[i]->flags &= ~EOF_DRUM_NOTE_FLAG_Y_COMBO;	//Remove yellow cymbal/tom combo status
				tp->note[i]->flags &= ~EOF_DRUM_NOTE_FLAG_B_COMBO;	//Remove blue cymbal/tom combo status
				tp->note[i]->flags &= ~EOF_DRUM_NOTE_FLAG_G_COMBO;	//Remove green cymbal/tom combo status
			}
			if(tp->note[i]->accent)
			{	//If the note has any gems with accent status
				eof_set_accent_at_legacy_note_pos(tp, tp->note[i]->pos, tp->note[i]->accent, 1);	//Apply accent status to such gems on any notes in other difficulties at that position
			}
		}//For each note in the drum track
	}//If the track being cleaned is a drum track
}

int eof_legacy_track_add_star_power(EOF_LEGACY_TRACK * tp, unsigned long start_pos, unsigned long end_pos)
{
	if(tp && (tp->star_power_paths < EOF_MAX_PHRASES))
	{	//If the maximum number of star power phrases for this track hasn't already been defined
		tp->star_power_path[tp->star_power_paths].start_pos = start_pos;
		tp->star_power_path[tp->star_power_paths].end_pos = end_pos;
		tp->star_power_path[tp->star_power_paths].flags = 0;
		tp->star_power_path[tp->star_power_paths].name[0] = '\0';
		tp->star_power_paths++;
		return 1;	//Return success
	}
	return 0;	//Return error
}

void eof_legacy_track_delete_star_power(EOF_LEGACY_TRACK * tp, unsigned long index)
{
	unsigned long i;

	if(!tp || (index >= tp->star_power_paths))
		return;

	tp->star_power_path[index].name[0] = '\0';	//Empty the name string
	for(i = index; i < tp->star_power_paths - 1; i++)
	{
		memcpy(&tp->star_power_path[i], &tp->star_power_path[i + 1], sizeof(EOF_PHRASE_SECTION));
	}
	tp->star_power_paths--;
}

int eof_legacy_track_add_solo(EOF_LEGACY_TRACK * tp, unsigned long start_pos, unsigned long end_pos)
{
	if(tp && (tp->solos < EOF_MAX_PHRASES))
	{	//If the maximum number of solo phrases for this track hasn't already been defined
		tp->solo[tp->solos].start_pos = start_pos;
		tp->solo[tp->solos].end_pos = end_pos;
		tp->solo[tp->solos].flags = 0;
		tp->solo[tp->solos].name[0] = '\0';
		tp->solos++;
		return 1;	//Return success
	}
	return 0;	//Return error
}

void eof_legacy_track_delete_solo(EOF_LEGACY_TRACK * tp, unsigned long index)
{
	unsigned long i;

	if(!tp || (index >= tp->solos))
		return;

	tp->solo[index].name[0] = '\0';	//Empty the name string
	for(i = index; i < tp->solos - 1; i++)
	{
		memcpy(&tp->solo[i], &tp->solo[i + 1], sizeof(EOF_PHRASE_SECTION));
	}
	tp->solos--;
}

EOF_LYRIC * eof_vocal_track_add_lyric(EOF_VOCAL_TRACK * tp)
{
	if(tp && (tp->lyrics < EOF_MAX_LYRICS))
	{
		tp->lyric[tp->lyrics] = malloc(sizeof(EOF_LYRIC));
		if(tp->lyric[tp->lyrics])
		{
			memset(tp->lyric[tp->lyrics], 0, sizeof(EOF_LYRIC));
			tp->lyrics++;
			return tp->lyric[tp->lyrics - 1];
		}
	}
	return NULL;
}

void eof_vocal_track_delete_lyric(EOF_VOCAL_TRACK * tp, unsigned long lyric)
{
	unsigned long i;

	if(tp && (lyric < tp->lyrics))
	{
		free(tp->lyric[lyric]);
		for(i = lyric; i < tp->lyrics - 1; i++)
		{
			tp->lyric[i] = tp->lyric[i + 1];
		}
		tp->lyrics--;
	}
}

void eof_vocal_track_sort_lyrics(EOF_VOCAL_TRACK * tp)
{
	if(tp)
	{
		qsort(tp->lyric, (size_t)tp->lyrics, sizeof(EOF_LYRIC *), eof_song_qsort_lyrics);
	}
}

long eof_fixup_previous_lyric(EOF_VOCAL_TRACK * tp, unsigned long lyric)
{
	if(tp)
	{
		if(lyric > 0)
		{
			return lyric - 1;
		}
	}
	return -1;
}

long eof_fixup_next_lyric(EOF_VOCAL_TRACK * tp, unsigned long lyric)
{
	if(tp)
	{
		if(lyric + 1 < tp->lyrics)
		{
			return lyric + 1;
		}
	}
	return -1;
}

void eof_vocal_track_fixup_lyrics(EOF_SONG *sp, unsigned long track, int sel)
{
	unsigned long i, j, tracknum;
	int lc;
	long next;
	EOF_VOCAL_TRACK * tp;

	if(!sp || (track >= sp->tracks))
	{
		return;
	}
	tracknum = sp->track[track]->tracknum;
	tp = sp->vocal_track[tracknum];
	if(!sel)
	{
		if(eof_selection.current < tp->lyrics)
		{
			eof_selection.multi[eof_selection.current] = 0;
		}
		eof_selection.current = EOF_MAX_NOTES - 1;
	}
	for(i = tp->lyrics; i > 0; i--)
	{
		/* fix selections */
		if(tp->lyric[i-1]->pos == eof_selection.current_pos)
		{
			eof_selection.current = i-1;
		}
		if(tp->lyric[i-1]->pos == eof_selection.last_pos)
		{
			eof_selection.last = i-1;
		}

		/* delete certain notes */
		if((tp->lyric[i-1]->pos < sp->tags->ogg[eof_selected_ogg].midi_offset) || (tp->lyric[i-1]->pos >= eof_chart_length))
		{
			eof_vocal_track_delete_lyric(tp, i-1);
		}

		else
		{
			/* make sure there are no 0-length notes */
			if(tp->lyric[i-1]->length <= 0)
			{
				tp->lyric[i-1]->length = 1;
			}

			/* make sure note doesn't extend past end of song */
			if(tp->lyric[i-1]->pos + tp->lyric[i-1]->length >= eof_chart_length)
			{
				tp->lyric[i-1]->length = eof_chart_length - tp->lyric[i-1]->pos;
			}

			/* compare this note to the next one of the same type
			   to make sure they don't overlap */
			next = eof_fixup_next_lyric(tp, i-1);
			if(next >= 0)
			{	//If there is a lyric after this lyric
				if(tp->lyric[i-1]->pos == tp->lyric[next]->pos)
				{	//And it is at the same position as this lyric, delete it
					eof_vocal_track_delete_lyric(tp, next);
				}
				else if(tp->lyric[i-1]->pos + tp->lyric[i-1]->length > tp->lyric[next]->pos - eof_min_note_distance)
				{	//Otherwise if it does not end at least the configured minimum distance before the next lyric starts
					if(tp->lyric[next]->pos - tp->lyric[i-1]->pos > eof_min_note_distance)
					{	//If there is enough distance between the lyrics to accommodate the minimum distance
						tp->lyric[i-1]->length = tp->lyric[next]->pos - tp->lyric[i-1]->pos - eof_min_note_distance;	//Apply it
					}
					else
					{	//Otherwise settle for 1ms
						tp->lyric[i-1]->length = 1;
					}
				}
			}
		}

		/* validate lyric text, ie. freestyle marker */
		eof_fix_lyric(tp,i-1);
	}

	/* make sure no lines overlap */
	for(i = 0; i < tp->lines; i++)
	{
		for(j = 0; j < tp->lines; j++)
		{
			if(j != i)
			{
				if((tp->line[i].end_pos >= tp->line[j].start_pos) && (tp->line[i].start_pos <= tp->line[j].end_pos))
				{
					tp->line[i].end_pos = tp->line[j].start_pos - 1;
				}
			}
		}
	}
	/* delete empty lines */
	for(i = tp->lines; i > 0; i--)
	{
		lc = 0;
		for(j = 0; j < tp->lyrics; j++)
		{
			if((tp->lyric[j]->pos >= tp->line[i-1].start_pos) && (tp->lyric[j]->pos <= tp->line[i-1].end_pos))
			{
				lc++;
			}
		}
		if(lc <= 0)
		{
			eof_vocal_track_delete_line(tp, i-1);
		}
	}
	if(!sel)
	{
		if(eof_selection.current < tp->lyrics)
		{
			eof_selection.multi[eof_selection.current] = 1;
		}
	}
}

int eof_vocal_track_add_star_power(EOF_VOCAL_TRACK * tp, unsigned long start_pos, unsigned long end_pos)
{
	unsigned long ctr;

	if(tp)
	{
		for(ctr = 0; ctr < tp->lines; ctr++)
		{	//For each line of lyrics
			if((tp->line[ctr].start_pos <= end_pos) && (tp->line[ctr].end_pos >= start_pos))
			{	//If this lyric line overlaps with the target time range
				tp->line[ctr].flags |= EOF_LYRIC_LINE_FLAG_OVERDRIVE;	//Set the overdrive flag
			}
		}
		return 1;	//Return success
	}
	return 0;	//Return error
}

int eof_vocal_track_add_line(EOF_VOCAL_TRACK * tp, unsigned long start_pos, unsigned long end_pos)
{
	if(tp && (tp->lines < EOF_MAX_LYRIC_LINES))
	{	//If the maximum number of lyric phrases hasn't already been defined
		tp->line[tp->lines].start_pos = start_pos;
		tp->line[tp->lines].end_pos = end_pos;
		tp->line[tp->lines].flags = 0;	//Ensure that a blank flag status is initialized
		tp->line[tp->lines].name[0] = '\0';
		tp->lines++;
		return 1;	//Return success
	}
	return 0;	//Return error
}

void eof_vocal_track_delete_line(EOF_VOCAL_TRACK * tp, unsigned long index)
{
	unsigned long i;

	if(!tp || (tp->lines == 0))	//If there are no lyric line phrases to delete
		return;			//Cancel this to avoid problems

	for(i = index; i < tp->lines - 1; i++)
	{
		memcpy(&tp->line[i], &tp->line[i + 1], sizeof(EOF_PHRASE_SECTION));
	}
	tp->lines--;
}

/* make sure notes don't overlap */
void eof_fixup_notes(EOF_SONG *sp)
{
	unsigned long j;

 	eof_log("eof_fixup_notes() entered", 1);

	if(sp)
	{
		for(j = 1; j < sp->tracks; j++)
		{
			eof_track_fixup_notes(sp, j, 1);
		}
	}
}

void eof_sort_notes(EOF_SONG *sp)
{
	unsigned long j;

 	eof_log("eof_sort_notes() entered", 1);

	if(sp)
	{
		for(j = 1; j < sp->tracks; j++)
		{
			eof_track_sort_notes(sp, j);
		}
	}
}

unsigned char eof_detect_difficulties(EOF_SONG * sp, unsigned long track)
{
	unsigned long i;
	unsigned char numdiffs = 5, note_type;

 	eof_log("eof_detect_difficulties() entered", 2);

	if(sp && (track < sp->tracks))
	{
		memset(eof_track_diff_populated_status, 0, sizeof(eof_track_diff_populated_status));
		eof_note_type_name[0][0] = ' ';
		eof_note_type_name[1][0] = ' ';
		eof_note_type_name[2][0] = ' ';
		eof_note_type_name[3][0] = ' ';
		eof_note_type_name[4][0] = ' ';
		eof_vocal_tab_name[0][0] = ' ';
		eof_dance_tab_name[0][0] = ' ';
		eof_dance_tab_name[1][0] = ' ';
		eof_dance_tab_name[2][0] = ' ';
		eof_dance_tab_name[3][0] = ' ';
		eof_dance_tab_name[4][0] = ' ';

		for(i = 0; i < eof_get_track_size(sp, track); i++)
		{
			if(sp->track[track]->track_format == EOF_VOCAL_TRACK_FORMAT)
			{
				if(sp->vocal_track[sp->track[track]->tracknum]->lyrics)
				{
					eof_vocal_tab_name[0][0] = '*';
					eof_track_diff_populated_status[0] = 1;
					break;
				}
			}
			else
			{
				note_type = eof_get_note_type(sp, track, i);
				eof_track_diff_populated_status[note_type] = 1;
				if(note_type >= numdiffs)
				{	//If this note's difficulty is the highest encountered in the track so far
					numdiffs = note_type + 1;
				}
				if(note_type < EOF_MAX_DIFFICULTIES)
				{	//If this note has a valid type (difficulty) in the traditional 5 difficulty system
					if(track == EOF_TRACK_DANCE)
					{	//If this is the dance track, update the dance track tabs
						eof_dance_tab_name[note_type][0] = '*';
					}
					else
					{	//Otherwise update the legacy track tabs
						eof_note_type_name[note_type][0] = '*';
					}
				}
			}
		}

		if(sp->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
		{	//If a pro guitar track is being processed
			EOF_PRO_GUITAR_TRACK *tp = sp->pro_guitar_track[sp->track[track]->tracknum];

			if(tp->note == tp->technote)
			{	//If tech view is in effect, the technotes counter is not yet up to date after a deletion operation
				tp->technotes = tp->notes;	//Update it
			}
			else
			{	//Otherwise if tech view is not in effect, the pgnotes counter is not yet up to date after a deletion operation
				tp->pgnotes = tp->notes;	//Update it
			}
			memset(eof_track_diff_populated_tech_note_status, 0, sizeof(eof_track_diff_populated_tech_note_status));
			for(i = 0; i < tp->technotes; i++)
			{
				eof_track_diff_populated_tech_note_status[tp->technote[i]->type] = 1;
			}
		}
	}

	return numdiffs;
}

int eof_lyric_is_freestyle(EOF_VOCAL_TRACK * tp, unsigned long lyricnumber)
{
	if((tp == NULL) || (lyricnumber >= tp->lyrics))
		return -1;	//Return error

	return eof_is_freestyle(tp->lyric[lyricnumber]->text);
}

int eof_is_freestyle(char *ptr)
{
	unsigned long ctr = 0;
	char c = 0;

	if(ptr == NULL)
		return -1;	//Return error

	for(ctr = 0; ctr < EOF_MAX_LYRIC_LENGTH; ctr++)
	{
		c = ptr[ctr];

		if(c == '\0')	//End of string
			break;

		if((c == '#') || (c == '^'))
			return 1;	//Return is freestyle
	}

	return 0;	//Return not freestyle
}

void eof_set_freestyle(char *ptr, char status)
{
// 	eof_log("eof_set_freestyle() entered");

	unsigned long ctr = 0, ctr2 = 0;
	char c = 0, style = 0;

	if(ptr == NULL)
		return;	//Return if input is invalid

	for(ctr = 0; ctr < EOF_MAX_LYRIC_LENGTH; ctr++)
	{
		c = ptr[ctr];

		if((c != '#') && (c != '^'))	//If this is not a freestyle character
		{
			ptr[ctr2] = c;				//keep it, otherwise it will be overwritten by the rest of the lyric text
			if(c == '\0')				//If the end of the string was just parsed
				break;					//Exit from loop
			ctr2++;						//Increment destination index into lyric string
		}
		else
		{
			if(!style)
				style = c;				//Remember this lyric's first freestyle character
		}
	}

	if(ctr == EOF_MAX_LYRIC_LENGTH)
	{	//If the lyric is not properly NULL terminated
		ptr[EOF_MAX_LYRIC_LENGTH] = '\0';	//Truncate the string at its max length
		ctr2 = EOF_MAX_LYRIC_LENGTH;
	}

//At this point, ctr2 is the NULL terminator's index into the string
	if(status)
	{
		if(!style)			//If the original string didn't have a particular freestyle marker
			style = '#';	//Default to using '#'

		if(ctr2 < EOF_MAX_LYRIC_LENGTH)	//If there is room to append the pound character
		{
			ptr[ctr2] = style;
			ptr[ctr2+1] = '\0';
		}
		else if(ctr2 > 0)		//If one byte of the string can be truncated to write the pound character
		{
			ptr[ctr2-1] = style;
		}
	}
}

void eof_fix_lyric(EOF_VOCAL_TRACK * tp, unsigned long lyricnumber)
{
	int result=0;

	if((tp == NULL) || (lyricnumber >= tp->lyrics))
		return;	//Return if input is invalid

	result=eof_lyric_is_freestyle(tp,lyricnumber);
	if(result >= 0)												//As long as there wasn't an error with the lyric
		eof_set_freestyle(tp->lyric[lyricnumber]->text,result);	//Rewrite lyric (if the NULL was missing, the string will be corrected)
}

void eof_toggle_freestyle(EOF_VOCAL_TRACK * tp, unsigned long lyricnumber)
{
	if((tp == NULL) || (lyricnumber >= tp->lyrics))
		return;	//Return if input is invalid

	if(eof_lyric_is_freestyle(tp,lyricnumber))				//If the lyric is freestyle
		eof_set_freestyle(tp->lyric[lyricnumber]->text,0);	//Remove its freestyle character(s)
	else													//Otherwise
		eof_set_freestyle(tp->lyric[lyricnumber]->text,1);	//Rewrite as freestyle
}

double eof_calc_beat_length(EOF_SONG *sp, unsigned long beat)
{
	unsigned long ctr;
	unsigned num = 4, den = 4;
	double ms = 500.0;

	if(!sp || (beat >= sp->beats))
		return 0.0;	//Invalid parameters

	for(ctr = 0; ctr <= beat; ctr++)
	{	//For each beat, including the target
		(void) eof_get_ts(sp, &num, &den, ctr);	//Lookup any time signature defined at the beat
	}

	ms = (60000.0 / (60000000.0 / (double)sp->beat[beat]->ppqn));	//Get the length of a quarter note based on the tempo in effect
	if(sp->tags->accurate_ts)
	{	//If the user enabled the accurate time signatures song property
		ms /= (double)den / 4.0;	//Translate this into the length of a beat based on the time signature in effect
	}
	return ms;
}

char eof_check_flags_at_legacy_note_pos(EOF_LEGACY_TRACK *tp, unsigned notenum, unsigned long flag)
{
// 	eof_log("eof_check_flags_at_legacy_note_pos() entered");

	unsigned long ctr;

	if((tp == NULL) || (notenum >= tp->notes))
		return 0;

	for(ctr = notenum; ctr < tp->notes; ctr++)
	{	//For each note starting with the one specified
		if(tp->note[ctr]->pos != tp->note[notenum]->pos)
			break;	//If there are no more notes at the specified note's position, stop looking
		if(tp->note[ctr]->flags & flag)
		{	//If the note has the specified flag
			return 1;	//Return match
		}
	}

	return 0;	//Return no match
}

void eof_set_flags_at_legacy_note_pos(EOF_LEGACY_TRACK *tp, unsigned notenum, unsigned long flag, char operation, char startpoint)
{
// 	eof_log("eof_set_flags_at_legacy_note_pos() entered");

	unsigned long ctr;

	if((tp == NULL) || (notenum >= tp->notes))
		return;

	if(!startpoint)
	{	//If the calling function wanted to start at the first note with the referenced note's position
		while((notenum > 0) && (tp->note[notenum]->pos == tp->note[notenum - 1]->pos))
		{	//If there is a prior note and it is at the same timestamp
			notenum--;	//Re-focus the start of this procedure to that note
		}
	}

	for(ctr = notenum; ctr < tp->notes; ctr++)
	{	//For each note starting with the one specified
		if(tp->note[ctr]->pos != tp->note[notenum]->pos)
			break;	//If there are no more notes at the specified note's position, stop looking
		if(operation == 0)
		{	//If the calling function indicated to clear the flag
			tp->note[ctr]->flags &= (~flag);
		}
		else if(operation == 1)
		{	//If the calling function indicated to set the flag
			tp->note[ctr]->flags |= flag;
		}
		else if(operation == 2)
		{	//If the calling function indicated to toggle the flag
			tp->note[ctr]->flags ^= flag;
		}
	}
}

void eof_set_accent_at_legacy_note_pos(EOF_LEGACY_TRACK *tp, unsigned long pos, unsigned long mask, char operation)
{
// 	eof_log("eof_set_accent_at_legacy_note_pos() entered");

	unsigned long ctr;

	if(tp == NULL)
		return;

	for(ctr = 0; ctr < tp->notes; ctr++)
	{	//For each note starting with the one specified
		if(tp->note[ctr]->pos > pos)
			break;	//If there are no more notes at the specified note's position, stop looking

		if(tp->note[ctr]->pos == pos)
		{	//If this note is at the target position
			if(operation == 0)
			{	//If the calling function indicated to clear the specified accent bits
				tp->note[ctr]->accent &= (~mask);
			}
			else if(operation == 1)
			{	//If the calling function indicated to set the specified accent bits
				tp->note[ctr]->accent |= mask;
			}
			else if(operation == 2)
			{	//If the calling function indicated to toggle the specified accent bits
				tp->note[ctr]->accent ^= mask;
			}
		}
	}
}

int eof_load_song_string_pf(char *const buffer, PACKFILE *fp, const size_t buffersize)
{
	unsigned long ctr = 0, stringsize = 0;
	int inputc = 0;

	if((fp == NULL) || ((buffer == NULL) && (buffersize != 0)))
		return 1;	//Return error

	stringsize = pack_igetw(fp);

	for(ctr = 0; ctr < stringsize; ctr++)
	{
		inputc = pack_getc(fp);
		if(inputc == EOF)		//If the end of file was reached
			break;			//stop reading characters
		if((size_t)ctr + 1 < buffersize)	//If the buffer can accommodate this character
			buffer[ctr] = inputc;	//store it
	}

	if(buffersize != 0)
	{	//Skip the termination of the buffer if none was presented
		if((size_t)ctr < buffersize)
			buffer[ctr] = '\0';		//NULL terminate the buffer
		else
			buffer[buffersize-1] = '\0';	//NULL terminate the end of the buffer
	}

	return 0;	//Return success
}

int eof_song_add_track(EOF_SONG * sp, EOF_TRACK_ENTRY * trackdetails)
{
	EOF_LEGACY_TRACK *ptr = NULL;
	EOF_VOCAL_TRACK *ptr2 = NULL;
	EOF_TRACK_ENTRY *ptr3 = NULL;
	EOF_PRO_GUITAR_TRACK *ptr4 = NULL;
	unsigned long count=0;

 	eof_log("eof_song_add_track() entered", 2);

	if((sp == NULL) || (trackdetails == NULL))
		return 0;	//Return error

	if(sp->tracks < EOF_TRACKS_MAX)
	{	//For each native track
		ptr3 = malloc(sizeof(EOF_TRACK_ENTRY));
		if(ptr3 == NULL)
			return 0;	//Return error

		//Insert new track structure in the appropriate track type array
		switch(trackdetails->track_format)
		{
			case EOF_LEGACY_TRACK_FORMAT:
				count = sp->legacy_tracks;
				ptr = malloc(sizeof(EOF_LEGACY_TRACK));
				if(ptr == NULL)
				{
					free(ptr3);
					return 0;	//Return error
				}
				ptr->notes = 0;
				ptr->solos = 0;
				ptr->star_power_paths = 0;
				ptr->trills = 0;
				ptr->tremolos = 0;
				ptr->sliders = 0;
				if(trackdetails->flags & EOF_TRACK_FLAG_SIX_LANES)
				{	//Open strum and fifth drum lane are tracked as a sixth lane
					ptr->numlanes = 6;
				}
				else if(trackdetails->track_type == EOF_TRACK_DANCE)
				{	//For now, the dance track is 4 lanes
					ptr->numlanes = 4;
				}
				else
				{	//Otherwise, all legacy tracks will be 5 lanes by default
					ptr->numlanes = 5;
				}
				ptr->parent = ptr3;
				sp->legacy_track[sp->legacy_tracks] = ptr;
				sp->legacy_tracks++;
			break;
			case EOF_VOCAL_TRACK_FORMAT:
				count = sp->vocal_tracks;
				ptr2 = malloc(sizeof(EOF_VOCAL_TRACK));
				if(ptr2 == NULL)
				{
					free(ptr3);
					return 0;	//Return error
				}
				ptr2->lyrics = 0;
				ptr2->lines = 0;
				ptr2->parent = ptr3;
				sp->vocal_track[sp->vocal_tracks] = ptr2;
				sp->vocal_tracks++;
			break;
			case EOF_PRO_KEYS_TRACK_FORMAT:
			break;
			case EOF_PRO_GUITAR_TRACK_FORMAT:
				count = sp->pro_guitar_tracks;
				ptr4 = malloc(sizeof(EOF_PRO_GUITAR_TRACK));
				if(ptr4 == NULL)
				{
					free(ptr3);
					return 0;	//Return error
				}
				memset(ptr4, 0, sizeof(EOF_PRO_GUITAR_TRACK));	//Initialize memory block to 0 to avoid crashes when not explicitly setting counters that were newly added to the pro guitar structure
				if((trackdetails->track_type == EOF_TRACK_PRO_BASS_22) || (trackdetails->track_type == EOF_TRACK_PRO_GUITAR_22))
				{	//If this is a 22 fret track
					ptr4->numfrets = 22;	//Set 22 as the default max fret (ie. Squier guitar)
				}
				else
				{	//Otherwise assume a default max fret of 17 (ie. Mustang controller)
					ptr4->numfrets = 17;
				}
				if((trackdetails->track_type == EOF_TRACK_PRO_BASS) || (trackdetails->track_type == EOF_TRACK_PRO_BASS_22))
				{
					ptr4->numstrings = 4;	//By default, set a pro bass track to 4 strings
					ptr4->arrangement = 4;	//And the arrangement type is "Bass"
				}
				else
				{
					ptr4->numstrings = 6;	//Otherwise, assume a 6 string guitar
				}
				if(ptr4->numstrings > EOF_TUNING_LENGTH)	//Ensure that the tuning array is large enough
				{
					free(ptr3);
					free(ptr4);
					return 0;	//Return error
				}
				ptr4->defaulttone[0] = '\0';	//Ensure this string is emptied
				ptr4->parent = ptr3;
				ptr4->note = ptr4->pgnote;		//Put the regular pro guitar note array into effect
				ptr4->ignore_tuning = 1;		//By default, chord lookups will ignore the tuning and capo and assume standard tuning
				sp->pro_guitar_track[sp->pro_guitar_tracks] = ptr4;
				sp->pro_guitar_tracks++;
			break;
			case EOF_PRO_VARIABLE_LEGACY_TRACK_FORMAT:	//Variable Lane Legacy (not implemented yet)
			break;
			default:
				eof_log("\tError:  Invalid track format", 1);
				free(ptr3);
			return 0;	//Return error
		}

		//Insert new track structure in the main track array and copy details
		ptr3->tracknum = count;
		ptr3->track_format = trackdetails->track_format;
		ptr3->track_behavior = trackdetails->track_behavior;
		ptr3->track_type = trackdetails->track_type;
		(void) ustrcpy(ptr3->name, trackdetails->name);
		(void) ustrcpy(ptr3->altname, trackdetails->altname);
		ptr3->difficulty = trackdetails->difficulty;
		ptr3->numdiffs = 5;	//By default, all tracks are limited to the original 5 difficulties
		ptr3->flags = trackdetails->flags;
		if(sp->tracks == 0)
		{	//If this is the first track being added, ensure that sp->track[0] is inserted
			sp->track[0] = NULL;
			sp->tracks++;
		}
		sp->track[sp->tracks] = ptr3;
		sp->tracks++;
	}//For each native track
	return 1;	//Return success
}

int eof_song_delete_track(EOF_SONG * sp, unsigned long track)
{
	unsigned long ctr;

 	eof_log("eof_song_delete_track() entered", 1);

	if((sp == NULL) || (track >= sp->tracks) || (sp->track[track] == NULL))
		return 0;	//Return error

	//Remove the track from the appropriate track type array
	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->tracknum >= sp->legacy_tracks) || (sp->legacy_track[sp->track[track]->tracknum] == NULL))
				return 0;	//Cannot remove a legacy track that doesn't exist
			free(sp->legacy_track[sp->track[track]->tracknum]);
			for(ctr = sp->track[track]->tracknum; ctr + 1 < sp->legacy_tracks; ctr++)
			{
				sp->legacy_track[ctr] = sp->legacy_track[ctr + 1];
			}
			sp->legacy_tracks--;
		break;
		case EOF_VOCAL_TRACK_FORMAT:
			if((sp->track[track]->tracknum >= sp->vocal_tracks) || (sp->vocal_track[sp->track[track]->tracknum] == NULL))
				return 0;	//Cannot remove a vocal track that doesn't exist
			free(sp->vocal_track[sp->track[track]->tracknum]);
			for(ctr = sp->track[track]->tracknum; ctr + 1 < sp->vocal_tracks; ctr++)
			{
				sp->vocal_track[ctr] = sp->vocal_track[ctr + 1];
			}
			sp->vocal_tracks--;
		break;
		case EOF_PRO_KEYS_TRACK_FORMAT:
		break;
		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if((sp->track[track]->tracknum >= sp->pro_guitar_tracks) || (sp->pro_guitar_track[sp->track[track]->tracknum] == NULL))
				return 0;	//Cannot remove a pro guitar track that doesn't exist
			free(sp->pro_guitar_track[sp->track[track]->tracknum]);
			for(ctr = sp->track[track]->tracknum; ctr + 1 < sp->pro_guitar_tracks; ctr++)
			{
				sp->pro_guitar_track[ctr] = sp->pro_guitar_track[ctr + 1];
			}
			sp->pro_guitar_tracks--;
		break;
		case EOF_PRO_VARIABLE_LEGACY_TRACK_FORMAT:	//Variable Lane Legacy (not implemented yet)
		break;
	}

	//Remove the track from the main track array
	free(sp->track[track]);
	for(ctr = track; ctr + 1 < sp->tracks; ctr++)
	{
		sp->track[ctr] = sp->track[ctr + 1];
	}
	sp->tracks--;
	if(sp->tracks == 1)
	{	//If only the dummy track[0] entry remains
		sp->tracks = 0;	//Drop it so sp->tracks equaling 0 can reflect that there are no tracks
	}
	return 1;	//Return success
}

void eof_read_pro_guitar_note(EOF_PRO_GUITAR_NOTE *ptr, PACKFILE *fp)
{
	unsigned long ctr, bitmask;

	if(!ptr || !fp)
		return;	//Invalid parameters

	(void) eof_load_song_string_pf(ptr->name,fp,EOF_NAME_LENGTH);	//Read the note's name
	(void) pack_getc(fp);																//Read the chord's number (not supported yet)
	ptr->type = pack_getc(fp);		//Read the note's difficulty
	ptr->note = pack_getc(fp);		//Read note bitflags
	ptr->ghost = pack_getc(fp);	//Read ghost bitflags
	for(ctr = 0, bitmask = 1; ctr < 8; ctr++, bitmask <<= 1)
	{	//For each of the 8 bits in the note bitflag
		if(ptr->note & bitmask)
		{	//If this bit is set
			ptr->frets[ctr] = pack_getc(fp);	//Read this string's fret value
		}
		else
		{	//Write a default value of 0
			ptr->frets[ctr] = 0;
		}
	}
	ptr->legacymask = pack_getc(fp);	//Read the legacy note bitmask
	ptr->pos = pack_igetl(fp);			//Read note position
	ptr->length = pack_igetl(fp);		//Read note length
	ptr->flags = pack_igetl(fp);		//Read note flags
	if(ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_RS_NOTATION)
	{	//If this note has bend string or slide ending fret definitions
		if((ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_SLIDE_UP) || (ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_SLIDE_DOWN))
		{	//If this is a slide note
			ptr->slideend = pack_getc(fp);	//Read the slide's ending fret
		}
		if(ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_BEND)
		{	//If this is a bend note
			ptr->bendstrength = pack_getc(fp);	//Read the bend's strength
		}
	}
	if(ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_UNPITCH_SLIDE)
	{	//If this is an unpitched slide note
		ptr->unpitchend = pack_getc(fp);	//Read the unpitched slide's ending fret
	}
	if(ptr->flags & EOF_NOTE_FLAG_EXTENDED)
	{	//If this note uses any extended flags
		ptr->eflags = pack_igetl(fp);		//Read extended flags
		ptr->flags &= ~EOF_NOTE_FLAG_EXTENDED;	//Clear this flag, it won't be updated again until the project is saved/loaded
	}
}

int eof_load_song_pf(EOF_SONG * sp, PACKFILE * fp)
{
	unsigned char inputc;
	unsigned long inputl,count,ctr,ctr2,bitmask;
	unsigned long track_count,track_ctr;
	unsigned long section_type_count,section_type_ctr,section_type,section_count,section_ctr,section_start,section_end;
	unsigned long custom_data_count,custom_data_ctr,custom_data_size,custom_data_id;
	EOF_TRACK_ENTRY temp={0, 0, 0, 0, "", "", 0, 5, 0};
	char name[EOF_SECTION_NAME_LENGTH + 1];	//Used to load note/section names (section names are currently longer)

	#define EOFNUMINISTRINGTYPES 9
	char *inistringbuffer[EOFNUMINISTRINGTYPES] = {NULL};
	unsigned long const inistringbuffersize[EOFNUMINISTRINGTYPES]={0,0,256,256,256,0,32,512,256};
		//Store the buffer information of each of the INI strings to simplify the loading code
		//This buffer can be updated without redesigning the entire load function, just add logic for loading the new string type
	#define EOFNUMINIBOOLEANTYPES 13
	char *inibooleanbuffer[EOFNUMINIBOOLEANTYPES] = {NULL};
		//Store the pointers to each of the boolean type INI settings (number 0 is reserved) to simplify the loading code
	#define EOFNUMININUMBERTYPES 5
	unsigned long *ininumberbuffer[EOFNUMININUMBERTYPES] = {NULL};
		//Store the pointers to each of the 5 number type INI settings (number 0 is reserved) to simplify the loading code

	unsigned long data_block_type, num_midi_tracks, numevents;
	char buffer[100] = {0};
	struct eof_MIDI_data_track *trackptr;
	struct eof_MIDI_data_event *eventptr, *eventhead, *eventtail;
	unsigned char numdiffs;
	char unshare_drum_phrasing;
	EOF_PRO_GUITAR_TRACK *tp = NULL;

 	eof_log("eof_load_song_pf() entered", 1);

	if((sp == NULL) || (fp == NULL))
		return 0;	//Return failure

	unshare_drum_phrasing = sp->tags->unshare_drum_phrasing;	//Store this outside the project structure until load completes, so that both drum tracks' phrases can be loaded
	sp->tags->unshare_drum_phrasing = 1;

	//Populate the arrays with addresses for variables here instead of during declaration to avoid compiler warnings
	inistringbuffer[2] = sp->tags->artist;
	inistringbuffer[3] = sp->tags->title;
	inistringbuffer[4] = sp->tags->frettist;
	inistringbuffer[6] = sp->tags->year;
	inistringbuffer[7] = sp->tags->loading_text;
	inistringbuffer[8] = sp->tags->album;
	inibooleanbuffer[1] = &sp->tags->lyrics;
	inibooleanbuffer[2] = &sp->tags->eighth_note_hopo;
	inibooleanbuffer[3] = &sp->tags->eof_fret_hand_pos_1_pg;
	inibooleanbuffer[4] = &sp->tags->eof_fret_hand_pos_1_pb;
	inibooleanbuffer[5] = &sp->tags->tempo_map_locked;
	inibooleanbuffer[6] = &sp->tags->double_bass_drum_disabled;
	inibooleanbuffer[7] = &sp->tags->click_drag_disabled;
	inibooleanbuffer[8] = &sp->tags->rs_chord_technique_export;
	inibooleanbuffer[9] = &unshare_drum_phrasing;
	inibooleanbuffer[10] = &sp->tags->highlight_unsnapped_notes;
	inibooleanbuffer[11] = &sp->tags->accurate_ts;
	inibooleanbuffer[12] = &sp->tags->highlight_arpeggios;
	ininumberbuffer[2] = &sp->tags->difficulty;

	/* read chart properties */
	sp->tags->revision = pack_igetl(fp);	//Read project revision number
	inputc = pack_getc(fp);					//Read timing format
	if(inputc == 1)
	{
		eof_log("Error: Delta timing is not yet supported", 1);
		return 0;	//Return failure
	}
	(void) pack_igetl(fp);		//Read time division (not supported yet)

	/* read song properties */
	count = pack_igetw(fp);			//Read the number of INI strings
	for(ctr=0; ctr<count; ctr++)
	{	//For each INI string in the project
		inputc = pack_getc(fp);		//Read the type of INI string
		if((inputc < EOFNUMINISTRINGTYPES) && (inistringbuffer[inputc] != NULL))
		{	//If EOF can store the INI setting natively
			(void) eof_load_song_string_pf(inistringbuffer[inputc], fp, (size_t)inistringbuffersize[inputc]);
		}
		else
		{	//If it is not natively supported or is otherwise a custom INI setting
			if(sp->tags->ini_settings < EOF_MAX_INI_SETTINGS)
			{	//If this INI setting can be stored
				(void) eof_load_song_string_pf(sp->tags->ini_setting[sp->tags->ini_settings],fp,sizeof(sp->tags->ini_setting[0]));
				sp->tags->ini_settings++;
			}
		}
	}

	count = pack_igetw(fp);			//Read the number of INI booleans
	for(ctr=0; ctr<count; ctr++)
	{	//For each INI boolean in the project
		inputc = pack_getc(fp);		//Read the type of INI boolean
		if(((inputc & 0x7F) < EOFNUMINIBOOLEANTYPES) && (inibooleanbuffer[(inputc & 0x7F)] != NULL))
		{	//Mask out the true/false bit to determine if it is a supported boolean setting
			*inibooleanbuffer[(inputc & 0x7F)] = ((inputc & 0xF0) != 0);	//Store the true/false status into the appropriate project variable
		}
	}

	count = pack_igetw(fp);			//Read the number of INI numbers
	for(ctr=0; ctr<count; ctr++)
	{	//For each INI number in the project
		inputc = pack_getc(fp);		//Read the type of INI number
		inputl = pack_igetl(fp);	//Read the INI number
		if((inputc < EOFNUMININUMBERTYPES) && (ininumberbuffer[inputc] != NULL))
		{	//If EOF can store the INI number natively
			*ininumberbuffer[inputc] = inputl;
		}
	}

	/* read chart data */
	count = pack_igetw(fp);			//Read the number of OGG profiles
	sp->tags->oggs = 0;
	for(ctr=0; ctr<count; ctr++)
	{	//For each OGG profile in the project
		if(sp->tags->oggs < EOF_MAX_OGGS)
		{	//IF EOF can store this OGG profile
			(void) eof_load_song_string_pf(sp->tags->ogg[sp->tags->oggs].filename,fp,256);	//Read the OGG filename
			(void) eof_load_song_string_pf(NULL,fp,0);				//Parse past the original audio file name (not supported yet)
			(void) eof_load_song_string_pf(sp->tags->ogg[sp->tags->oggs].description,fp,0);	//Read the OGG profile description string
			sp->tags->ogg[sp->tags->oggs].midi_offset = pack_igetl(fp);	//Read the profile's MIDI delay
			(void) pack_igetl(fp);									//Read the OGG profile flags (not supported yet)
			sp->tags->oggs++;
		}
	}

	count = pack_igetl(fp);					//Read the number of beats
	if(!eof_song_resize_beats(sp, count))	//Resize the beat array accordingly
	{
		eof_log("Error:  Couldn't resize beat array", 1);
		return 0;	//Return error upon failure to do so
	}
	for(ctr=0; ctr<count; ctr++)
	{	//For each beat in the project
		sp->beat[ctr]->ppqn = pack_igetl(fp);		//Read the beat's tempo
		sp->beat[ctr]->pos = pack_igetl(fp);		//Read the beat's position (milliseconds or delta ticks)
		sp->beat[ctr]->fpos = sp->beat[ctr]->pos;	//For now, assume the position is in milliseconds and copy to the fpos variable as-is (if a custom data block containing floating point timings is found, they will be loaded)
		sp->beat[ctr]->flags = pack_igetl(fp);		//Read the beat's flags
		sp->beat[ctr]->key = pack_getc(fp);			//Read the beat's key signature
	}

	count = pack_igetl(fp);				//Read the number of text events
	if(!eof_song_resize_text_events(sp, count))	//Resize the text event array accordingly
	{
		eof_log("Error:  Couldn't resize text event array", 1);
		return 0;	//Return error upon failure to do so
	}
	for(ctr=0; ctr<count; ctr++)
	{	//For each text event in the project
		(void) eof_load_song_string_pf(sp->text_event[ctr]->text,fp,256);	//Read the text event string
		sp->text_event[ctr]->beat = pack_igetl(fp);		//Read the text event's beat number
		sp->text_event[ctr]->track = pack_igetw(fp);	//Read the text event's associated track number
		sp->text_event[ctr]->flags = pack_igetw(fp);	//Read the text event's flags
	}

	custom_data_count = pack_igetl(fp);		//Read the number of custom data blocks
	for(custom_data_ctr = 0; custom_data_ctr < custom_data_count; custom_data_ctr++)
	{	//For each custom data block in the project
		custom_data_size = pack_igetl(fp);	//Read the size of the custom data block
		data_block_type = pack_igetl(fp);	//Read the data block type
		if(data_block_type == 1)
		{	//If this is a linked list of raw MIDI track data
			unsigned char mididataflags, deltatimings = 0;
			unsigned long timedivision = 0;

			num_midi_tracks = pack_igetw(fp);	//Read the number of tracks to read
			mididataflags = pack_getc(fp);		//Read the raw MIDI data block flags
			(void) pack_getc(fp);	//Read the reserved byte (not used)
			for(ctr = 0; ctr < num_midi_tracks; ctr++)
			{	//For each of the tracks to read
				eventhead = eventtail = NULL;	//The event linked list begins empty
				trackptr = malloc(sizeof(struct eof_MIDI_data_track));
				if(!trackptr)
				{
					eof_log("Error:  Couldn't allocate memory for raw MIDI data", 1);
					return 0;	//Memory allocation error
				}
				(void) eof_load_song_string_pf(buffer, fp, sizeof(buffer));	//Read the MIDI track name
				if(buffer[0] == '\0')
				{	//If there is no track name
					trackptr->trackname = NULL;
				}
				else
				{	//If there is a track name
					trackptr->trackname = malloc(strlen(buffer) + 1);	//Allocate enough memory to duplicate this string
					if(!trackptr->trackname)
					{
						eof_log("Error:  Couldn't allocate memory for raw MIDI track name", 1);
						free(trackptr);
						return 0;	//Memory allocation error
					}
					strcpy(trackptr->trackname, buffer);
				}
				(void) eof_load_song_string_pf(buffer, fp, sizeof(buffer));	//Read the description string
				if(buffer[0] == '\0')
				{	//If there is no description
					trackptr->description = NULL;
				}
				else
				{	//If there is a description
					trackptr->description = malloc(strlen(buffer) + 1);	//Allocate enough memory to duplicate this string
					if(!trackptr->description)
					{
						free(trackptr->trackname);
						free(trackptr);
						eof_log("Error:  Couldn't allocate memory for raw MIDI track description", 1);
						return 0;	//Memory allocation error
					}
					strcpy(trackptr->description, buffer);
				}
				numevents = pack_igetl(fp);	//Read the number of events for this track
				timedivision = 0;	//This will be zero unless the stored track contains it
				if(mididataflags & 1)
				{	//If the MIDI data block's flags indicated that delta timings are allowed
					deltatimings = pack_getc(fp);	//Read the byte indicating whether delta timings are present for this track
					if(deltatimings)
					{	//If this track is storing a delta time for each stored event
						timedivision = pack_igetl(fp);	//Read the time division associated with the timings
					}
				}
				trackptr->timedivision = timedivision;
				for(ctr2 = 0; ctr2 < numevents; ctr2++)
				{	//For each of the events to read
					eventptr = malloc(sizeof(struct eof_MIDI_data_event));
					if(!eventptr)
					{
						free(trackptr->trackname);
						free(trackptr->description);
						free(trackptr);
						eof_log("Error:  Couldn't allocate memory for raw MIDI text event", 1);
						while(eventhead)
						{	//Release all raw MIDI events
							eventtail = eventhead->next;	//Save the address of the next link
							free(eventhead);				//Free the head link
							eventhead = eventtail;			//Point to the next link
						}
						return 0;	//Memory allocation error
					}
					(void) eof_load_song_string_pf(buffer, fp, sizeof(buffer));	//Read the timestamp string
					(void) sscanf(buffer, "%99lf", &eventptr->realtime);		//Convert to double floating point (sscanf is width limited to prevent buffer overflow)
					eventptr->stringtime = malloc(strlen(buffer) + 1);			//Allocate enough memory to store the timestamp string
					if(!eventptr->stringtime)
					{
						free(trackptr->trackname);
						free(trackptr->description);
						free(trackptr);
						free(eventptr);
						eof_log("Error:  Couldn't allocate memory for raw MIDI timestamp", 1);
						while(eventhead)
						{	//Release all raw MIDI events
							eventtail = eventhead->next;	//Save the address of the next link
							free(eventhead);				//Free the head link
							eventhead = eventtail;			//Point to the next link
						}
						return 0;	//Memory allocation error
					}
					strcpy(eventptr->stringtime, buffer);	//Store the timestamp string
					eventptr->deltatime = 0;
					if(deltatimings)
					{	//If this track is storing a delta time for each stored event
						eventptr->deltatime = pack_igetl(fp);	//Read the event's delta time
					}
					eventptr->size = pack_igetw(fp);	//Get the size of this event's data
					eventptr->data = malloc((size_t)eventptr->size);	//Allocate enough memory to store the event data
					if(!eventptr->data)
					{
						free(trackptr->trackname);
						free(trackptr->description);
						free(trackptr);
						free(eventptr->stringtime);
						free(eventptr);
						eof_log("Error:  Couldn't allocate memory for raw MIDI event data", 1);
						while(eventhead)
						{	//Release all raw MIDI events
							eventtail = eventhead->next;	//Save the address of the next link
							free(eventhead);				//Free the head link
							eventhead = eventtail;			//Point to the next link
						}
						return 0;	//Memory allocation error
					}
					(void) pack_fread(eventptr->data, (long)eventptr->size, fp);	//Read the event's data
					eventptr->next = NULL;
					if(eventhead == NULL)
					{	//If the list is empty
						eventhead = eventptr;	//The new link is now the first link in the list
					}
					else if(eventtail != NULL)
					{	//If there is already a link at the end of the list
						eventtail->next = eventptr;	//Point it forward to the new link
					}
					eventtail = eventptr;	//The new link is the new tail of the list
				}
				trackptr->events = eventhead;	//Store the events linked list in the track link
				trackptr->next = NULL;
				eof_MIDI_add_track(sp, trackptr);	//Store the track link in the EOF_SONG structure
			}//For each of the tracks to read
		}//If this is a linked list of raw MIDI track data
		else if(data_block_type == 2)
		{	//If this is a set of floating point beat timings
			for(ctr = 0; ctr < sp->beats; ctr++)
			{	//For each beat in the project
				(void) eof_load_song_string_pf(buffer, fp, sizeof(buffer));		//Read the timestamp string
				(void) sscanf(buffer, "%99lf", &sp->beat[ctr]->fpos);			//Convert to double floating point (sscanf is width limited to prevent buffer overflow)
				sp->beat[ctr]->pos = sp->beat[ctr]->fpos + 0.5;					//Round this up to the nearest millisecond to get the integer timestamp of the beat
			}
			sp->fpbeattimes = 1;	//Have eof_init_after_load() skip the recalculation of beat timings, since the original floating point timings were loaded
		}
		else
		{	//Otherwise, skip over the unknown data block
			for(ctr=4; ctr<custom_data_size; ctr++)
			{	//For each byte in the custom data block (accounting for the four byte data block type that was read)
				(void) pack_getc(fp);	//Read the data (not supported yet)
			}
		}
	}//For each custom data block in the project

	/* read track data */
	track_count = pack_igetl(fp);		//Read the number of tracks
	for(track_ctr=0; track_ctr<track_count; track_ctr++)
	{	//For each track in the project
		(void) eof_load_song_string_pf(temp.name,fp,sizeof(temp.name));	//Read the track name
		temp.track_format = pack_getc(fp);		//Read the track format
		temp.track_behavior = pack_getc(fp);	//Read the track behavior
		temp.track_type = pack_getc(fp);		//Read the track type
		temp.difficulty = pack_getc(fp);		//Read the track difficulty level
		temp.flags = pack_igetl(fp);			//Read the track flags
		(void) pack_igetw(fp);					//Read the track compliance flags (not supported yet)
		temp.tracknum=0;						//Ignored, it will be set in eof_song_add_track()
		if(temp.flags & EOF_TRACK_FLAG_ALT_NAME)
		{	//If this track has an alternate name
			(void) eof_load_song_string_pf(temp.altname,fp,sizeof(temp.altname));	//Read the alternate track name
		}

		if(track_ctr != 0)
		{	//Add track to project
			if(eof_song_add_track(sp, &temp) == 0)	//Add the track
			{
				eof_log("Error:  Couldn't add track", 1);
				return 0;	//Return error upon failure
			}
		}
		switch(temp.track_format)
		{	//Perform the appropriate logic to load this format of track
			case 0:	//The global track only has section data
			break;
			case EOF_LEGACY_TRACK_FORMAT:	//Legacy (non pro guitar, non pro bass, non pro keys, pro or non pro drums)
				sp->legacy_track[sp->legacy_tracks-1]->numlanes = pack_getc(fp);	//Read the number of lanes/keys/etc. used in this track
				count = pack_igetl(fp);	//Read the number of notes in this track
				if(count > EOF_MAX_NOTES)
				{
					(void) snprintf(eof_log_string, sizeof(eof_log_string) - 1, "Error: Unsupported number of notes in track %lu.  Aborting", track_ctr);
					allegro_message("%s", eof_log_string);
					eof_log(eof_log_string, 1);
					return 0;
				}
				eof_track_resize(sp, sp->tracks-1,count);	//Resize the note array
				for(ctr=0; ctr<count; ctr++)
				{	//For each note in this track
					(void) eof_load_song_string_pf(sp->legacy_track[sp->legacy_tracks-1]->note[ctr]->name,fp,EOF_NAME_LENGTH);	//Read the note's name
					sp->legacy_track[sp->legacy_tracks-1]->note[ctr]->type = pack_getc(fp);		//Read the note's difficulty
					sp->legacy_track[sp->legacy_tracks-1]->note[ctr]->note = pack_getc(fp);		//Read note bitflags
					sp->legacy_track[sp->legacy_tracks-1]->note[ctr]->pos = pack_igetl(fp);		//Read note position
					sp->legacy_track[sp->legacy_tracks-1]->note[ctr]->length = pack_igetl(fp);	//Read note length
					sp->legacy_track[sp->legacy_tracks-1]->note[ctr]->flags = pack_igetl(fp);	//Read note flags
				}
			break;
			case EOF_VOCAL_TRACK_FORMAT:	//Vocal
				(void) pack_getc(fp);	//Read the tone set number assigned to this track (not supported yet)
				count = pack_igetl(fp);	//Read the number of notes in this track
				if(count > EOF_MAX_LYRICS)
				{
					(void) snprintf(eof_log_string, sizeof(eof_log_string) - 1, "Error: Unsupported number of lyrics in track %lu.  Aborting", track_ctr);
					allegro_message("%s", eof_log_string);
					eof_log(eof_log_string, 1);
					return 0;
				}
				eof_track_resize(sp, sp->tracks-1,count);	//Resize the lyrics array
				for(ctr=0; ctr<count; ctr++)
				{	//For each lyric in this track
					(void) eof_load_song_string_pf(sp->vocal_track[sp->vocal_tracks-1]->lyric[ctr]->text,fp,EOF_MAX_LYRIC_LENGTH);	//Read the lyric text
					(void) pack_getc(fp);	//Read lyric set number (not supported yet)
					sp->vocal_track[sp->vocal_tracks-1]->lyric[ctr]->note = pack_getc(fp);		//Read lyric pitch
					sp->vocal_track[sp->vocal_tracks-1]->lyric[ctr]->pos = pack_igetl(fp);		//Read lyric position
					sp->vocal_track[sp->vocal_tracks-1]->lyric[ctr]->length = pack_igetl(fp);	//Read lyric length
					(void) pack_igetl(fp);	//Read lyric flags (not supported yet)
				}
			break;
			case EOF_PRO_KEYS_TRACK_FORMAT:	//Pro Keys
				allegro_message("Error: Pro Keys not supported yet.  Aborting");
				eof_log("Error: Pro Keys not supported yet.  Aborting", 1);
			return 0;
			case EOF_PRO_GUITAR_TRACK_FORMAT:	//Pro Guitar/Bass
				tp = sp->pro_guitar_track[sp->pro_guitar_tracks-1];	//Simplify, tp is the pointer to the pro guitar track being parsed
				numdiffs = 5;		//By default, assume there are 5 difficulties used in the track
				tp->numfrets = pack_getc(fp);	//Read the number of frets used in this track
				count = pack_getc(fp);	//Read the number of strings used in this track
				if(count > 8)
				{
					(void) snprintf(eof_log_string, sizeof(eof_log_string) - 1, "Error: Unsupported number of strings in track %lu.  Aborting", track_ctr);
					allegro_message("%s", eof_log_string);
					eof_log(eof_log_string, 1);
					return 0;
				}
				tp->numstrings = count;
				if(tp->numstrings > EOF_TUNING_LENGTH)
				{	//Prevent overflow
					(void) snprintf(eof_log_string, sizeof(eof_log_string) - 1, "Unsupported number of strings in track %lu.  Aborting", track_ctr);
					allegro_message("%s", eof_log_string);
					eof_log(eof_log_string, 1);
					return 0;
				}
				for(ctr=0; ctr < tp->numstrings; ctr++)
				{	//For each string
					tp->tuning[ctr] = pack_getc(fp);	//Read the string's tuning
				}
				count = pack_igetl(fp);	//Read the number of notes in this track
				if(count > EOF_MAX_NOTES)
				{
					(void) snprintf(eof_log_string, sizeof(eof_log_string) - 1, "Error: Unsupported number of notes in track %lu.  Aborting", track_ctr);
					allegro_message("%s", eof_log_string);
					eof_log(eof_log_string, 1);
					return 0;
				}
				eof_track_resize(sp, sp->tracks-1,count);	//Resize the note array
				for(ctr=0; ctr<count; ctr++)
				{	//For each note in this track
					eof_read_pro_guitar_note(tp->note[ctr], fp);	//Read the data for this note from the project file
					if(tp->note[ctr]->type >= numdiffs)
					{	//If this note's difficulty is the highest encountered in the track so far
						numdiffs = tp->note[ctr]->type + 1;	//Track it
					}
				}//For each note in this track
				tp->pgnotes = tp->notes;			//Store the size of the regular pro guitar note array
				tp->parent->numdiffs = numdiffs;	//Update the track's difficulty count
			break;
			case EOF_PRO_VARIABLE_LEGACY_TRACK_FORMAT:	//Variable Lane Legacy (not implemented yet)
				allegro_message("Error: Variable lane not supported yet.  Aborting");
			return 0;
			default://Unknown track type
				allegro_message("Error: Unsupported track type.  Aborting");
				eof_log("Error: Unsupported track type.  Aborting", 1);
			return 0;
		}

		section_type_count = pack_igetw(fp);	//Read the number of types of sections defined for this track
		for(section_type_ctr=0; section_type_ctr<section_type_count; section_type_ctr++)
		{	//For each type of section in this track
			section_type = pack_igetw(fp);		//Read the type of section this is
			section_count = pack_igetl(fp);		//Read the number of instances of this type of section there is
			for(section_ctr=0; section_ctr<section_count; section_ctr++)
			{	//For each instance of the specified section
				(void) eof_load_song_string_pf(name,fp,EOF_SECTION_NAME_LENGTH + 1);	//Parse past the section name
				inputc = pack_getc(fp);								//Read the section's associated difficulty
				section_start = pack_igetl(fp);						//Read the start timestamp of the section
				section_end = pack_igetl(fp);						//Read the end timestamp of the section
				inputl = pack_igetl(fp);							//Read the section flags

				//Perform the appropriate logic to load this type of section
				switch(track_ctr)
				{
					case 0:		//Sections defined in track 0 are global
						switch(section_type)
						{
							case EOF_BOOKMARK_SECTION:		//Bookmark section
								(void) eof_track_add_section(sp,0,EOF_BOOKMARK_SECTION,0,section_start,section_end,inputl,NULL);
							break;
							case EOF_FRET_CATALOG_SECTION:	//Fret Catalog section
								(void) eof_track_add_section(sp,0,EOF_FRET_CATALOG_SECTION,inputc,section_start,section_end,inputl,name);	//For fret catalog sections, the flag represents the associated track number
							break;
						}
					break;

					default:	//Read track-specific sections
						(void) eof_track_add_section(sp,track_ctr,section_type,inputc,section_start,section_end,inputl,name);
					break;
				}
			}
		}//For each type of section in this track

		custom_data_count = pack_igetl(fp);		//Read the number of custom data blocks
		for(custom_data_ctr=0; custom_data_ctr<custom_data_count; custom_data_ctr++)
		{	//For each custom data block in the track
			custom_data_size = pack_igetl(fp);	//Read the size of the custom data block
			custom_data_id = pack_igetl(fp);	//Read the ID of the custom data block
			switch(custom_data_id)
			{
				case 2:		//Pro guitar finger array custom data block ID
					if(custom_data_size < 4)
					{	//This is invalid, the size needed to have included the 4 byte ID
						allegro_message("Error:  Invalid custom data block size (finger definitions).  Aborting");
						eof_log("Error:  Invalid custom data block size (finger definitions).  Aborting", 1);
						return 0;
					}
					custom_data_size -= 4;	//Subtract the size of the block ID, which was already read
					if(sp->track[track_ctr]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
					{	//Ensure this logic only runs for a pro guitar track
						tp = sp->pro_guitar_track[sp->pro_guitar_tracks-1];	//Redundant assignment of tp to resolve a false positive with Coverity
						for(ctr = 0; ctr < eof_get_track_size(sp, track_ctr); ctr++)
						{	//For each note in this track
							for(ctr2 = 0, bitmask = 1; ctr2 < tp->numstrings; ctr2++, bitmask <<= 1)
							{	//For each supported string in the track
								if(tp->note[ctr]->note & bitmask)
								{	//If this string is used
									tp->note[ctr]->finger[ctr2] = pack_getc(fp);	//Read the finger value for this note
								}
							}
						}
					}
					else
					{	//Corrupt file
						allegro_message("Error: Invalid pro guitar finger array data block.  Aborting");
						eof_log("Error: Invalid pro guitar finger array data block.  Aborting", 1);
						return 0;
					}
				break;

				case 3:		//Pro guitar track arrangement type
					if(custom_data_size != 5)
					{	//This data block is expected to be 5 bytes long
						allegro_message("Error:  Invalid custom data block size (arrangement type).  Aborting");
						eof_log("Error:  Invalid custom data block size (arrangement type).  Aborting", 1);
						return 0;
					}
					if(sp->track[track_ctr]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
					{	//Ensure this logic only runs for a pro guitar track
						tp = sp->pro_guitar_track[sp->pro_guitar_tracks-1];	//Redundant assignment of tp to resolve a false positive with Coverity
						tp->arrangement = pack_getc(fp);	//Read the track arrangement type
					}
				break;

				case 4:		//Pro guitar track tuning not honored
					if(custom_data_size != 5)
					{	//This data block is expected to be 5 bytes long
						allegro_message("Error:  Invalid custom data block size (track tuning not honored).  Aborting");
						eof_log("Error:  Invalid custom data block size (track tuning not honored).  Aborting", 1);
						return 0;
					}
					if(sp->track[track_ctr]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
					{	//Ensure this logic only runs for a pro guitar track
						tp = sp->pro_guitar_track[sp->pro_guitar_tracks-1];	//Redundant assignment of tp to resolve a false positive with Coverity
						tp->ignore_tuning = pack_getc(fp);	//Read the option of whether the chord detection does not honor the track's defined tuning
					}
				break;

//				case 5:		//Pro guitar vibrato speeds (this data block is deprecated but is reserved for backwards compatibility with 1.8RC9 era release candidates)
//				break;

				case 6:		//Pro guitar capo position
					if(custom_data_size != 5)
					{	//This data block is expected to be 5 bytes long
						allegro_message("Error:  Invalid custom data block size (capo position).  Aborting");
						eof_log("Error:  Invalid custom data block size (capo position).  Aborting", 1);
						return 0;
					}
					if(sp->track[track_ctr]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
					{	//Ensure this logic only runs for a pro guitar track
						tp = sp->pro_guitar_track[sp->pro_guitar_tracks-1];	//Redundant assignment of tp to resolve a false positive with Coverity
						tp->capo = pack_getc(fp);	//Read the capo position
					}
				break;

				case 7:		//Pro guitar tech notes
					if(custom_data_size < 8)
					{	//This data block is expected to be at least 8 bytes long
						allegro_message("Error:  Invalid custom data block size (tech notes).  Aborting");
						eof_log("Error:  Invalid custom data block size (tech notes).  Aborting", 1);
						return 0;
					}
					if(sp->track[track_ctr]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
					{	//Ensure this logic only runs for a pro guitar track
						tp = sp->pro_guitar_track[sp->pro_guitar_tracks-1];	//Redundant assignment of tp to resolve a false positive with Coverity
						tp->technotes = pack_igetl(fp);	//Read the number of tech notes
						for(ctr = 0; ctr < tp->technotes; ctr++)
						{	//For each tech note in the custom data block
							tp->technote[ctr] = malloc(sizeof(EOF_PRO_GUITAR_NOTE));	//Allocate memory for the tech note
							if(!tp->technote[ctr])
							{
								allegro_message("Error:  Unable to allocate memory for (tech notes).  Aborting");
								eof_log("Error:  Unable to allocate memory for (tech notes).  Aborting", 1);
								return 0;
							}
							memset(tp->technote[ctr], 0, sizeof(EOF_PRO_GUITAR_NOTE));
							eof_read_pro_guitar_note(tp->technote[ctr], fp);	//Read the tech note
						}
					}
				break;

				case 8:		//Accent note bitmasks
					if(custom_data_size < 5)
					{	//This data block is expected to be at least 5 bytes long
						allegro_message("Error:  Invalid custom data block size (accent note bitmasks).  Aborting");
						eof_log("Error:  Invalid custom data block size (accent note bitmasks).  Aborting", 1);
						return 0;
					}
					if(sp->track[track_ctr]->track_format == EOF_LEGACY_TRACK_FORMAT)
					{	//Ensure this logic only runs for a legacy track
						for(ctr = 0; ctr < eof_get_track_size(sp, track_ctr); ctr++)
						{	//For each note in this track
							sp->legacy_track[sp->legacy_tracks-1]->note[ctr]->accent = pack_getc(fp);		//Read note accent bitmask
						}
					}
				break;

				default:	//Unknown custom data block ID
					if(custom_data_size < 4)
					{	//This is invalid, the size needed to have included the 4 byte ID
						allegro_message("Error:  Invalid custom data block size (unknown custom data block).  Aborting");
						eof_log("Error:  Invalid custom data block size (unknown custom data block).  Aborting", 1);
						return 0;
					}
					custom_data_size -= 4;	//Subtract the size of the block ID, which was already read
					for(ctr=0; ctr<custom_data_size; ctr++)
					{	//For each byte in the custom data block
						(void) pack_getc(fp);	//Read the data (ignoring it)
					}
				break;
			}
		}//For each custom data block in the track
	}//For each track in the project
	sp->tags->unshare_drum_phrasing = unshare_drum_phrasing;	//After all tracks have been formally loaded, store this value into the project to optionally override drum phrase handling
	return 1;	//Return success
}

int eof_track_add_section(EOF_SONG * sp, unsigned long track, unsigned long sectiontype, unsigned char difficulty, unsigned long start, unsigned long end, unsigned long flags, char *name)
{
	unsigned long count,tracknum;	//Used to de-obfuscate the track handling
	unsigned long ctr;

 	eof_log("eof_track_add_section() entered", 2);

	if((sp == NULL) || ((track != 0) && (track >= sp->tracks)))
		return 0;	//Return error

	if(track == 0)
	{	//Allow global sections to be added
		switch(sectiontype)
		{
			case EOF_BOOKMARK_SECTION:
				if(end < EOF_MAX_BOOKMARK_ENTRIES)
				{	//If EOF can store this bookmark
					sp->bookmark_pos[end] = start;
				}
			return 1;
			case EOF_FRET_CATALOG_SECTION:
				if(sp->catalog->entries < EOF_MAX_CATALOG_ENTRIES)
				{
					sp->catalog->entry[sp->catalog->entries].track = flags;		//For now, EOF still numbers tracks starting from number 0
					sp->catalog->entry[sp->catalog->entries].type = difficulty;	//Store the fret catalog section's associated difficulty
					sp->catalog->entry[sp->catalog->entries].start_pos = start;
					sp->catalog->entry[sp->catalog->entries].end_pos = end;
					if(name == NULL)
					{
						sp->catalog->entry[sp->catalog->entries].name[0] = '\0';
					}
					else
					{
						(void) ustrcpy(sp->catalog->entry[sp->catalog->entries].name, name);
					}
					sp->catalog->entries++;
				}
			return 1;
			default:	//Unknown global section type
			return 0;	//Return error
		}
	}

	tracknum = sp->track[track]->tracknum;
	switch(sectiontype)
	{	//Perform the appropriate logic to add this type of section
		case EOF_SOLO_SECTION:
		return eof_track_add_solo(sp, track, start, end);

		case EOF_SP_SECTION:
		return eof_track_add_star_power_path(sp, track, start, end);

		case EOF_LYRIC_PHRASE_SECTION:	//Lyric Phrase section
			if(sp->track[track]->track_format == EOF_VOCAL_TRACK_FORMAT)
			{	//Lyric phrases are only valid for vocal tracks
				return eof_vocal_track_add_line(sp->vocal_track[tracknum], start, end);
			}
		return 0;	//Return error

		case EOF_YELLOW_CYMBAL_SECTION:	//Yellow cymbal section (not supported yet)
			if(sp->track[track]->track_behavior == EOF_DRUM_TRACK_BEHAVIOR)
			{	//Cymbal sections are only valid for drum tracks
			}
		break;
		case EOF_BLUE_CYMBAL_SECTION:	//Blue cymbal section (not supported yet)
			if(sp->track[track]->track_behavior == EOF_DRUM_TRACK_BEHAVIOR)
			{	//Cymbal sections are only valid for drum tracks
			}
		break;
		case EOF_GREEN_CYMBAL_SECTION:	//Green cymbal section (not supported yet)
			if(sp->track[track]->track_behavior == EOF_DRUM_TRACK_BEHAVIOR)
			{	//Cymbal sections are only valid for drum tracks
			}
		break;

		case EOF_TRILL_SECTION:
		return eof_track_add_trill(sp, track, start, end);

		case EOF_ARPEGGIO_SECTION:	//Arpeggio section
			if(sp->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
			{
				count = sp->pro_guitar_track[tracknum]->arpeggios;
				if(count < EOF_MAX_PHRASES)
				{	//If EOF can store the arpeggio section
					sp->pro_guitar_track[tracknum]->arpeggio[count].start_pos = start;
					sp->pro_guitar_track[tracknum]->arpeggio[count].end_pos = end;
					sp->pro_guitar_track[tracknum]->arpeggio[count].flags = flags;	//Specifies whether the arpeggio exports as an arpeggio or a normal handshape
					if(name == NULL)
					{
						sp->pro_guitar_track[tracknum]->arpeggio[count].name[0] = '\0';
					}
					else
					{
						(void) ustrcpy(sp->pro_guitar_track[tracknum]->arpeggio[count].name, name);
					}
					if((unsigned char)difficulty == 0xFF)
					{	//Beta versions of EOF 1.8 (up to beta 15) stored arpeggios without a specified difficulty, re-assign them to Expert
						difficulty = EOF_NOTE_AMAZING;
					}
					sp->pro_guitar_track[tracknum]->arpeggio[count].difficulty = difficulty;
					sp->pro_guitar_track[tracknum]->arpeggios++;
				}
				return 1;
			}
		break;

		case EOF_TRAINER_SECTION:	//Pro trainer section (not supported yet)
		break;
		case EOF_CUSTOM_MIDI_NOTE_SECTION:	//Custom MIDI note section (not supported yet)
		break;
		case EOF_PREVIEW_SECTION:	//Preview audio section (not supported yet)
		break;

		case EOF_TREMOLO_SECTION:
		return eof_track_add_tremolo(sp, track, start, end, difficulty);

		case EOF_SLIDER_SECTION:
			if((sp->track[track]->track_behavior == EOF_GUITAR_TRACK_BEHAVIOR) && (sp->track[track]->track_format == EOF_LEGACY_TRACK_FORMAT))
			{	//Only legacy guitar tracks are able to use this type of section
				count = sp->legacy_track[tracknum]->sliders;
				if(count < EOF_MAX_PHRASES)
				{	//If EOF can store the slider section
					sp->legacy_track[tracknum]->slider[count].start_pos = start;
					sp->legacy_track[tracknum]->slider[count].end_pos = end;
					sp->legacy_track[tracknum]->slider[count].flags = 0;
					if(name == NULL)
					{
						sp->legacy_track[tracknum]->slider[count].name[0] = '\0';
					}
					else
					{
						(void) ustrcpy(sp->legacy_track[tracknum]->slider[count].name, name);
					}
					sp->legacy_track[tracknum]->sliders++;
				}
				return 1;
			}
		break;
		case EOF_FRET_HAND_POS_SECTION:	//Fret hand position
			if(sp->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
			{
				count = sp->pro_guitar_track[tracknum]->handpositions;
				if(count < EOF_MAX_NOTES)
				{	//If EOF can store the fret hand position
					if(end && (end <= 21))
					{	//If the fret hand position is valid (greater than zero and at or below RS2's limit)
						sp->pro_guitar_track[tracknum]->handposition[count].start_pos = start;
						sp->pro_guitar_track[tracknum]->handposition[count].end_pos = end;	//This will store the fret number the fretting hand is at
						sp->pro_guitar_track[tracknum]->handposition[count].flags = 0;
						sp->pro_guitar_track[tracknum]->handposition[count].difficulty = difficulty;
						if(name == NULL)
						{
							sp->pro_guitar_track[tracknum]->handposition[count].name[0] = '\0';
						}
						else
						{
							(void) ustrcpy(sp->pro_guitar_track[tracknum]->handposition[count].name, name);
						}
						sp->pro_guitar_track[tracknum]->handpositions++;
						return 1;	//Return success
					}
				}
			}
		return 0;	//Return error
		case EOF_RS_POPUP_MESSAGE:	//Popup message
			if(sp->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
			{
				count = sp->pro_guitar_track[tracknum]->popupmessages;
				if(count < EOF_MAX_PHRASES)
				{	//If EOF can store the popup message
					sp->pro_guitar_track[tracknum]->popupmessage[count].start_pos = start;
					sp->pro_guitar_track[tracknum]->popupmessage[count].end_pos = end;	//This will store the fret number the fretting hand is at
					sp->pro_guitar_track[tracknum]->popupmessage[count].flags = flags;
					sp->pro_guitar_track[tracknum]->popupmessage[count].difficulty = 0;
					if(name == NULL)
					{
						sp->pro_guitar_track[tracknum]->popupmessage[count].name[0] = '\0';
					}
					else
					{
						(void) ustrcpy(sp->pro_guitar_track[tracknum]->popupmessage[count].name, name);
					}
					sp->pro_guitar_track[tracknum]->popupmessages++;
				}
				return 1;
			}
		break;
		case EOF_RS_TONE_CHANGE:	//Tone change
			if(sp->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
			{
				count = sp->pro_guitar_track[tracknum]->tonechanges;
				if(count < EOF_MAX_PHRASES)
				{	//If EOF can store the tone change
					if(name && (name[0] != '\0'))
					{	//If the tone name isn't NULL or empty, add the tone change to the project
						sp->pro_guitar_track[tracknum]->tonechange[count].start_pos = start;
						(void) ustrcpy(sp->pro_guitar_track[tracknum]->tonechange[count].name, name);
						for(ctr = 0; ctr < (unsigned long)strlen(sp->pro_guitar_track[tracknum]->tonechange[count].name); ctr++)
						{	//For each character in the name
							if(sp->pro_guitar_track[tracknum]->tonechange[count].name[ctr] == ' ')
							{	//If it's a space character
								sp->pro_guitar_track[tracknum]->tonechange[count].name[ctr] = '_';	//Replace it with an underscore
							}
						}
						if(end)
						{	//The project format uses this field as a boolean to identify if this is the default tone for the track
							strncpy(sp->pro_guitar_track[tracknum]->defaulttone, sp->pro_guitar_track[tracknum]->tonechange[count].name, EOF_SECTION_NAME_LENGTH);
						}
						sp->pro_guitar_track[tracknum]->tonechanges++;
					}
				}
				return 1;
			}
		break;
	}
	return 0;	//Return error
}

int eof_save_song_string_pf(char *buffer, PACKFILE *fp)
{
	unsigned long length = 0, ctr;

	if(fp == NULL)
		return 1;	//Return error

	if(buffer != NULL)
	{	//If the string isn't NULL
		length = ustrsize(buffer);	//Gets its length in bytes (allowing for Unicode string support)
	}

	(void) pack_iputw(length, fp);	//Write string length
	if(buffer != NULL)
	{	//Redundant check, but should make cppcheck happy
		for(ctr=0; ctr < length; ctr++)
		{
			(void) pack_putc(buffer[ctr], fp);
		}
	}

	return 0;	//Return success
}

void eof_write_pro_guitar_note(EOF_PRO_GUITAR_NOTE *ptr, PACKFILE *fp)
{
	unsigned long ctr, bitmask;

	if(!ptr || !fp)
		return;	//Invalid parameters

	(void) eof_save_song_string_pf(ptr->name, fp);	//Write the note's name
	(void) pack_putc(0, fp);				//Write the chord's number (not supported yet)
	(void) pack_putc(ptr->type, fp);		//Write the note's difficulty
	(void) pack_putc(ptr->note, fp);		//Write the note's bitflags
	(void) pack_putc(ptr->ghost, fp);		//Write the note's ghost bitflags
	for(ctr=0, bitmask=1; ctr < 8; ctr++, bitmask <<= 1)
	{	//For each of the 8 bits in the note bitflag
		if(ptr->note & bitmask)
		{	//If this bit is set
			(void) pack_putc(ptr->frets[ctr], fp);	//Write this string's fret value
		}
	}
	(void) pack_putc(ptr->legacymask, fp);	//Write the legacy note bitmask
	(void) pack_iputl(ptr->pos, fp);		//Write the note's position
	(void) pack_iputl(ptr->length, fp);		//Write the note's length
	if(ptr->eflags)
	{	//If this note uses any extended flags
		ptr->flags |= EOF_NOTE_FLAG_EXTENDED;	//Set this flag
	}
	else
	{
		ptr->flags &= ~EOF_NOTE_FLAG_EXTENDED;	//Clear this flag
	}
	(void) pack_iputl(ptr->flags, fp);		//Write the note's flags
	if(ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_RS_NOTATION)
	{	//If this note has bend string or slide ending fret definitions
		if((ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_SLIDE_UP) || (ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_SLIDE_DOWN))
		{	//If this is a slide note
			(void) pack_putc(ptr->slideend, fp);	//Write the slide's ending fret
		}
		if(ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_BEND)
		{	//If this is a bend note
			(void) pack_putc(ptr->bendstrength, fp);	//Write the bend's strength
		}
	}
	if(ptr->flags & EOF_PRO_GUITAR_NOTE_FLAG_UNPITCH_SLIDE)
	{	//If this is an unpitched slide note
		(void) pack_putc(ptr->unpitchend, fp);	//Write the unpitched slide's ending fret
	}
	if(ptr->flags & EOF_NOTE_FLAG_EXTENDED)
	{	//if this note uses any extended flags
		(void) pack_iputl(ptr->eflags, fp);		//Write the note's extended track flags
		ptr->flags &= ~EOF_NOTE_FLAG_EXTENDED;	//Clear this flag, it won't be updated again until the project is saved/loaded
	}
}

int eof_save_song(EOF_SONG * sp, const char * fn)
{
	PACKFILE * fp = NULL;
	char header[16] = {'E', 'O', 'F', 'S', 'O', 'N', 'H', 0};
	unsigned long count, ctr, ctr2, tracknum = 0;
	unsigned long track_count,track_ctr,bookmark_count,track_custom_block_count,bitmask,fingerdefinitions;
	char has_raw_midi_data, has_fp_beat_timings = 1;
	char has_solos,has_star_power,has_bookmarks,has_catalog,has_lyric_phrases,has_arpeggios,has_trills,has_tremolos,has_sliders,has_handpositions,has_popupmesages,has_fingerdefinitions,has_arrangement,has_tonechanges,ignore_tuning,has_capo,has_tech_notes,has_accent;

	#define EOFNUMINISTRINGTYPES 9
	char *inistringbuffer[EOFNUMINISTRINGTYPES] = {NULL};
		//Store the buffer information of each of the 12 INI strings to simplify the loading code
		//This buffer can be updated without redesigning the entire load function, just add logic for loading the new string type
	#define EOFNUMINIBOOLEANTYPES 13
	char *inibooleanbuffer[EOFNUMINIBOOLEANTYPES] = {NULL};
		//Store the pointers to each of the boolean type INI settings (number 0 is reserved) to simplify the loading code
	#define EOFNUMININUMBERTYPES 5
	unsigned long *ininumberbuffer[EOFNUMININUMBERTYPES] = {NULL};
		//Store the pointers to each of the 5 number type INI settings (number 0 is reserved) to simplify the loading code
	char unshare_drum_phrasing;

	EOF_PRO_GUITAR_TRACK *tp = NULL;
	char restore_tech_view;		//If tech view is in effect for a pro guitar track, it is temporarily disabled until after the track's notes have been written

 	eof_log("eof_save_song() entered", 1);

	if((sp == NULL) || (fn == NULL))
	{
		eof_log("\tError saving:  Invalid parameters", 1);
		return 0;	//Return error
	}

	(void) snprintf(eof_log_string, sizeof(eof_log_string) - 1, "Saving to file \"%s\"", fn);
	eof_log(eof_log_string, 1);

	unshare_drum_phrasing = sp->tags->unshare_drum_phrasing;	//Store this outside the project structure until save completes, so that both drum tracks' phrases can be written
	sp->tags->unshare_drum_phrasing = 1;

	//Populate the arrays with addresses for variables here instead of during declaration to avoid compiler warnings
	inistringbuffer[2] = sp->tags->artist;
	inistringbuffer[3] = sp->tags->title;
	inistringbuffer[4] = sp->tags->frettist;
	inistringbuffer[6] = sp->tags->year;
	inistringbuffer[7] = sp->tags->loading_text;
	inistringbuffer[8] = sp->tags->album;
	inibooleanbuffer[1] = &sp->tags->lyrics;
	inibooleanbuffer[2] = &sp->tags->eighth_note_hopo;
	inibooleanbuffer[3] = &sp->tags->eof_fret_hand_pos_1_pg;
	inibooleanbuffer[4] = &sp->tags->eof_fret_hand_pos_1_pb;
	inibooleanbuffer[5] = &sp->tags->tempo_map_locked;
	inibooleanbuffer[6] = &sp->tags->double_bass_drum_disabled;
	inibooleanbuffer[7] = &sp->tags->click_drag_disabled;
	inibooleanbuffer[8] = &sp->tags->rs_chord_technique_export;
	inibooleanbuffer[9] = &unshare_drum_phrasing;
	inibooleanbuffer[10] = &sp->tags->highlight_unsnapped_notes;
	inibooleanbuffer[11] = &sp->tags->accurate_ts;
	inibooleanbuffer[12] = &sp->tags->highlight_arpeggios;
	ininumberbuffer[2] = &sp->tags->difficulty;

	/* write file header */
	fp = pack_fopen(fn, "w");
	if(!fp)
	{
		(void) snprintf(eof_log_string, sizeof(eof_log_string) - 1, "\tError saving:  Cannot open output .eof file:  \"%s\"", strerror(errno));	//Get the Operating System's reason for the failure
		eof_log(eof_log_string, 1);
		return 0;	//Return error
	}
	(void) pack_fwrite(header, 16, fp);

	/* write chart properties */
	(void) pack_iputl(sp->tags->revision, fp);			//Write project revision number
	(void) pack_putc(0, fp);							//Write timing format
	(void) pack_iputl(EOF_DEFAULT_TIME_DIVISION, fp);	//Write time division (not supported yet)

	/* write song properties */
	//Count the number of INI strings to write (including custom strings)
	count = sp->tags->ini_settings;
	for(ctr=0; ctr < EOFNUMINISTRINGTYPES; ctr++)
	{
		if((inistringbuffer[ctr] != NULL) && (inistringbuffer[ctr][0] != '\0'))
		{	//If this native INI string is populated
			count++;
		}
	}
	(void) pack_iputw(count, fp);	//Write the number of INI strings
	for(ctr=0; ctr < EOFNUMINISTRINGTYPES; ctr++)
	{	//For each built-in INI string
		if((inistringbuffer[ctr] != NULL) && (inistringbuffer[ctr][0] != '\0'))
		{	//If this native INI string is populated
			(void) pack_putc(ctr, fp);	//Write the type of INI string
			(void) eof_save_song_string_pf(inistringbuffer[ctr], fp);	//Write the string
		}
	}
	for(ctr=0; ctr < sp->tags->ini_settings; ctr++)
	{	//For each custom INI string
		(void) pack_putc(0, fp);	//Write the "custom" INI string type
		(void) eof_save_song_string_pf(sp->tags->ini_setting[ctr], fp);	//Write the string
	}
	//Count the number of INI booleans to write
	count = 0;
	for(ctr=0; ctr < EOFNUMINIBOOLEANTYPES; ctr++)
	{
		if((inibooleanbuffer[ctr] != NULL) && (*inibooleanbuffer[ctr] != 0))
		{
			count++;
		}
	}
	(void) pack_iputw(count, fp);	//Write the number of INI booleans
	for(ctr=0; ctr < EOFNUMINIBOOLEANTYPES; ctr++)
	{	//For each boolean value
		if((inibooleanbuffer[ctr] != NULL) && (*inibooleanbuffer[ctr] != 0))
		{	//If this boolean value is nonzero
			(void) pack_putc(0x80 + ctr, fp);	//Write the type of INI boolean in the lower 7 bits with the MSB set to represent TRUE
		}
	}
	//Count the number of INI numbers to write
	count = 0;
	for(ctr=0; ctr < EOFNUMININUMBERTYPES; ctr++)
	{
		if(ininumberbuffer[ctr] != NULL)
		{
			count++;
		}
	}
	(void) pack_iputw(count, fp);	//Write the number of INI numbers
	for(ctr=0; ctr < EOFNUMININUMBERTYPES; ctr++)
	{
		if(ininumberbuffer[ctr] != NULL)
		{
			(void) pack_putc(ctr, fp);	//Write the type of INI number
			(void) pack_iputl(*ininumberbuffer[ctr], fp);	//Write the INI number
		}
	}

	/* write chart data */
	(void) pack_iputw(sp->tags->oggs, fp);	//Write the number of OGG profiles
	for(ctr=0; ctr < sp->tags->oggs; ctr++)
	{	//For each OGG profile in the project
		(void) eof_save_song_string_pf(sp->tags->ogg[ctr].filename, fp);	//Write the OGG filename string
		(void) eof_save_song_string_pf(NULL, fp);	//Write an empty original audio file name string (not supported yet)
		(void) eof_save_song_string_pf(sp->tags->ogg[ctr].description, fp);	//Write an OGG profile description string
		(void) pack_iputl(sp->tags->ogg[ctr].midi_offset, fp);	//Write the profile's MIDI delay
		(void) pack_iputl(0, fp);	//Write the profile's flags (not supported yet)
	}
	(void) pack_iputl(sp->beats, fp);	//Write the number of beats
	for(ctr=0; ctr < sp->beats; ctr++)
	{	//For each beat in the project
		(void) pack_iputl(sp->beat[ctr]->ppqn, fp);		//Write the beat's tempo
		(void) pack_iputl(sp->beat[ctr]->pos, fp);		//Write the beat's position (milliseconds or delta ticks)
		(void) pack_iputl(sp->beat[ctr]->flags, fp);	//Write the beat's flags
		(void) pack_putc(sp->beat[ctr]->key, fp);		//Write the beat's key signature
	}
	(void) pack_iputl(sp->text_events, fp);	//Write the number of text events
	for(ctr=0; ctr < sp->text_events; ctr++)
	{	//For each text event in the project
		(void) eof_save_song_string_pf(sp->text_event[ctr]->text, fp);	//Write the text event string
		(void) pack_iputl(sp->text_event[ctr]->beat, fp);	//Write the text event's associated beat number
		(void) pack_iputw(sp->text_event[ctr]->track, fp);	//Write the text event's associated track number
		(void) pack_iputw(sp->text_event[ctr]->flags, fp);	//Write the text event's flags
	}

	/* write custom data blocks */
//	(void) pack_iputl(0, fp);	//Write an empty custom data block
	has_raw_midi_data = 0;
	if(sp->midi_data_head)
	{	//If there is raw MIDI data being stored
		has_raw_midi_data = 1;
	}
	if(has_raw_midi_data || has_fp_beat_timings)
	{	//If writing data in a custom data block
		(void) pack_iputl(has_raw_midi_data + has_fp_beat_timings, fp);	//Write the number of custom data blocks
		if(has_raw_midi_data)
		{	//If there is raw MIDI data being stored, write it as a custom data block
			PACKFILE *tfp;	//Used to create a temp file containing the MIDI data block, so its size can easily be determined before dumping into the output project file
			struct eof_MIDI_data_track *trackptr = sp->midi_data_head;	//Point to the beginning of the track linked list
			struct eof_MIDI_data_event *eventptr;
			unsigned long filesize;

		//Parse the linked list to write the MIDI data to a temp file
			tfp = pack_fopen("rawmididata.tmp", "w");
			if(!tfp)
			{	//If the temp file couldn't be opened for writing
				eof_log("\tError creating temp file for raw MIDI data block", 1);
				(void) pack_fclose(fp);
				return 0;	//return error
			}
			for(ctr = 0, trackptr = sp->midi_data_head; trackptr != NULL; ctr++, trackptr = trackptr->next);	//Count the number of tracks in this list
			(void) pack_iputl(1, tfp);			//Write the data block ID (1 = Raw MIDI data)
			(void) pack_iputw(ctr, tfp);		//Write the number of tracks that will be stored in this data block
			(void) pack_putc(1, tfp);			//Write the raw MIDI data block flags (delta timings allowed)
			(void) pack_putc(0, tfp);			//Write the reserved byte (not used)
			trackptr = sp->midi_data_head;	//Point to the beginning of the track linked list
			while(trackptr != NULL)
			{	//For each track of event data
				(void) eof_save_song_string_pf(trackptr->trackname, tfp);		//Write the track name (this function allows for a NULL pointer)
				(void) eof_save_song_string_pf(trackptr->description, tfp);	//Write the description
				for(ctr = 0, eventptr = trackptr->events; eventptr != NULL; ctr++, eventptr = eventptr->next);	//Count the number of events in this track
				(void) pack_iputl(ctr, tfp);	//Write the number of events for this track
				if(trackptr->timedivision)
				{	//If this stored MIDI track has a time division defined
					(void) pack_putc(1, tfp);	//Write the delta timings present field to indicate each event will also have a delta time written
					(void) pack_iputl(trackptr->timedivision, tfp);	//And write the track's time division
				}
				else
				{
					(void) pack_putc(0, tfp);	//Write the delta timings present field to indicate that events will not include delta times
				}
				eventptr = trackptr->events;	//Point to the beginning of this track's event linked list
				while(eventptr != NULL)
				{	//For each event
					(void) eof_save_song_string_pf(eventptr->stringtime, tfp);	//Write the timestamp string
					if(trackptr->timedivision)
					{	//If this stored MIDI track has a time division defined
						(void) pack_iputl(eventptr->deltatime, tfp);	//Write the event's delta time
					}
					(void) pack_iputw(eventptr->size, tfp);	//Write the size of this event's data
					(void) pack_fwrite(eventptr->data, eventptr->size, tfp);	//Write this event's data
					eventptr = eventptr->next;	//Point to the next event
				}
				trackptr = trackptr->next;	//Point to the next track
			}
			(void) pack_fclose(tfp);	//Close temp file

		//Write the custom data block
			filesize = (unsigned long)file_size_ex("rawmididata.tmp");
			(void) pack_iputl(filesize, fp);	//Write the size of this data block
			tfp = pack_fopen("rawmididata.tmp", "r");
			if(!tfp)
			{	//If the temp file couldn't be opened for writing
				eof_log("\tError reading temp file for raw MIDI data block", 1);
				(void) pack_fclose(fp);
				return 0;	//return error
			}
			for(ctr = 0; ctr < filesize; ctr++)
			{	//For each byte in the temp file
				(void) pack_putc(pack_getc(tfp), fp);	//Copy the byte to the output project file
			}
			(void) pack_fclose(tfp);
			(void) delete_file("rawmididata.tmp");	//Delete the temp file
		}//If there is raw MIDI data being stored, write it as a custom data block
		if(has_fp_beat_timings)
		{	//If floating point beat timings are to be written
			char buffer[100] = {0};	//Will be used to store an ASCII representation of the beat timestamps
			PACKFILE *tfp;	//Used to create a temp file containing the beat timings, so its size can easily be determined before dumping into the output project file
			unsigned long filesize;

		//Write the beat timings to a temp file
			tfp = pack_fopen("beattimes.tmp", "w");
			if(!tfp)
			{	//If the temp file couldn't be opened for writing
				eof_log("\tError creating temp file for floating point beat timings data block", 1);
				(void) pack_fclose(fp);
				return 0;	//return error
			}
			for(ctr = 0; ctr < sp->beats; ctr++)
			{	//For each beat in the project
				(void) snprintf(buffer, sizeof(buffer) - 1, "%f", sp->beat[ctr]->fpos);	//Create a string representation of this beat's timestamp
				(void) eof_save_song_string_pf(buffer, tfp);		//Write timing string
			}
			(void) pack_fclose(tfp);	//Close temp file

		//Write the custom data block
			filesize = (unsigned long)file_size_ex("beattimes.tmp");
			(void) pack_iputl(filesize, fp);	//Write the size of this data block
			tfp = pack_fopen("beattimes.tmp", "r");
			if(!tfp)
			{	//If the temp file couldn't be opened for writing
				eof_log("\tError reading temp file for floating point beat timings data block", 1);
				(void) pack_fclose(fp);
				return 0;	//return error
			}
			for(ctr = 0; ctr < filesize; ctr++)
			{	//For each byte in the temp file
				(void) pack_putc(pack_getc(tfp), fp);	//Copy the byte to the output project file
			}
			(void) pack_fclose(tfp);
			(void) delete_file("beattimes.tmp");	//Delete the temp file
		}
	}//If writing data in a custom data block
	else
	{	//Otherwise write a debug custom data block
		(void) pack_iputl(1, fp);			//Write one data block
		(void) pack_iputl(4, fp);			//Write the size of this data block
		(void) pack_iputl(0xFFFFFFFF, fp);	//Write the data block ID (0xFFFFFFFF = Debug block)
	}

	/* write track data */
	//Count the number of bookmarks
	for(ctr=0,bookmark_count=0,has_bookmarks=0; ctr < EOF_MAX_BOOKMARK_ENTRIES; ctr++)
	{
		if(sp->bookmark_pos[ctr] > 0)
		{	//If this bookmark exists
			bookmark_count++;
			has_bookmarks = 1;
		}
	}
	//Count the number of catalog entries
	if((sp->catalog != NULL) && (sp->catalog->entries > 0))
	{
		has_catalog = 1;
	}
	else
	{
		has_catalog = 0;
	}
	//Determine how many tracks need to be written
	track_count = sp->tracks;
	if((track_count == 0) && (bookmark_count || sp->catalog->entries))
	{	//Ensure that a global track is written if necessary to accommodate bookmarks and catalog entries
		track_count = 1;
	}
	(void) pack_iputl(track_count, fp);	//Write the number of tracks
	for(track_ctr=0; track_ctr < track_count; track_ctr++)
	{	//For each track in the project
		restore_tech_view = 0;	//Reset this condition
		if(sp->track[track_ctr] != NULL)
		{	//Skip NULL tracks, such as track 0
			(void) eof_save_song_string_pf(sp->track[track_ctr]->name, fp);	//Write track name string
		}
		else
		{
			(void) eof_save_song_string_pf(NULL, fp);	//Write empty string
		}
		if(track_ctr == 0)
		{	//Write global track
			(void) pack_putc(0, fp);	//Write track format (global)
			(void) pack_putc(0, fp);	//Write track behavior (not used)
			(void) pack_putc(0, fp);	//Write track type (not used)
			(void) pack_putc(0, fp);	//Write track difficulty (not used)
			(void) pack_iputl(0, fp);	//Write global track flags (not supported yet)
			(void) pack_iputw(0xFFFF, fp);	//Write compliance flags (not used)
			//Write global track section type chunk
			(void) pack_iputw(has_bookmarks + has_catalog, fp);	//Write number of section types
			if(has_bookmarks)
			{	//Write bookmarks
				(void) pack_iputw(EOF_BOOKMARK_SECTION, fp);	//Write bookmark section type
				(void) pack_iputl(bookmark_count, fp);			//Write number of bookmarks
				for(ctr=0; ctr < EOF_MAX_BOOKMARK_ENTRIES; ctr++)
				{	//For each bookmark in the project
					if(sp->bookmark_pos[ctr] > 0)
					{
						(void) eof_save_song_string_pf(NULL, fp);		//Write an empty section name string (not supported yet)
						(void) pack_putc(0xFF, fp);					//Write an associated difficulty of "all difficulties"
						(void) pack_iputl(sp->bookmark_pos[ctr], fp);	//Write the bookmark's position
						(void) pack_iputl(ctr, fp);					//Write end position (bookmark number)
						(void) pack_iputl(0, fp);						//Write section flags (not used)
					}
				}
			}
			if(has_catalog)
			{	//Write fret catalog
				(void) pack_iputw(EOF_FRET_CATALOG_SECTION, fp);	//Write fret catalog section type
				(void) pack_iputl(sp->catalog->entries, fp);		//Write number of catalog entries
				for(ctr=0; ctr < sp->catalog->entries; ctr++)
				{	//For each fret catalog entry in the project
					(void) eof_save_song_string_pf(sp->catalog->entry[ctr].name, fp);	//Write the section name string
					(void) pack_putc(sp->catalog->entry[ctr].type, fp);					//Write the associated difficulty
					(void) pack_iputl(sp->catalog->entry[ctr].start_pos, fp);			//Write the catalog entry's position
					(void) pack_iputl(sp->catalog->entry[ctr].end_pos, fp);				//Write the catalog entry's end position
					(void) pack_iputl(sp->catalog->entry[ctr].track, fp);				//Write the flags (associated track number)
				}
			}
		}
		else
		{	//Write other tracks
			(void) pack_putc(sp->track[track_ctr]->track_format, fp);	//Write track format
			(void) pack_putc(sp->track[track_ctr]->track_behavior, fp);	//Write track behavior
			(void) pack_putc(sp->track[track_ctr]->track_type, fp);		//Write track type
			(void) pack_putc(sp->track[track_ctr]->difficulty, fp);		//Write track difficulty
			(void) pack_iputl(sp->track[track_ctr]->flags, fp);			//Write track flags
			(void) pack_iputw(0, fp);	//Write track compliance flags (not supported yet)

			if(sp->track[track_ctr]->flags & EOF_TRACK_FLAG_ALT_NAME)
			{	//If this track has an alternate name
				(void) eof_save_song_string_pf(sp->track[track_ctr]->altname, fp);	//Write alternate track name string
			}

			tracknum = sp->track[track_ctr]->tracknum;
			has_solos = has_star_power = has_lyric_phrases = has_arpeggios = has_trills = has_tremolos = has_sliders = has_handpositions = has_popupmesages = has_tonechanges = 0;
			switch(sp->track[track_ctr]->track_format)
			{	//Perform the appropriate logic to write this format of track
				case EOF_LEGACY_TRACK_FORMAT:	//Legacy (non pro guitar, non pro bass, non pro keys, pro or non pro drums)
					(void) pack_putc(sp->legacy_track[tracknum]->numlanes, fp);	//Write the number of lanes/keys/etc. used in this track
					(void) pack_iputl(sp->legacy_track[tracknum]->notes, fp);		//Write the number of notes in this track
					for(ctr=0; ctr < sp->legacy_track[tracknum]->notes; ctr++)
					{	//For each note in this track
						(void) eof_save_song_string_pf(sp->legacy_track[tracknum]->note[ctr]->name, fp);	//Write the note's name
						(void) pack_putc(sp->legacy_track[tracknum]->note[ctr]->type, fp);		//Write the note's difficulty
						(void) pack_putc(sp->legacy_track[tracknum]->note[ctr]->note, fp);		//Write the note's bitflags
						(void) pack_iputl(sp->legacy_track[tracknum]->note[ctr]->pos, fp);		//Write the note's position
						(void) pack_iputl(sp->legacy_track[tracknum]->note[ctr]->length, fp);	//Write the note's length
						(void) pack_iputl(sp->legacy_track[tracknum]->note[ctr]->flags, fp);	//Write the note's flags
					}
					//Write the section type chunk
					if(sp->legacy_track[tracknum]->solos)
					{
						has_solos = 1;
					}
					if(sp->legacy_track[tracknum]->star_power_paths)
					{
						has_star_power = 1;
					}
					if(sp->legacy_track[tracknum]->trills)
					{
						has_trills = 1;
					}
					if(sp->legacy_track[tracknum]->tremolos)
					{
						has_tremolos = 1;
					}
					if(sp->legacy_track[tracknum]->sliders)
					{
						has_sliders = 1;
					}
					(void) pack_iputw(has_solos + has_star_power + has_trills + has_tremolos + has_sliders, fp);	//Write number of section types
					if(has_solos)
					{	//Write solo sections
						(void) pack_iputw(EOF_SOLO_SECTION, fp);	//Write solo section type
						(void) pack_iputl(sp->legacy_track[tracknum]->solos, fp);	//Write number of solo sections for this track
						for(ctr=0; ctr < sp->legacy_track[tracknum]->solos; ctr++)
						{	//For each solo section in the track
							(void) eof_save_song_string_pf(NULL, fp);		//Write an empty section name string (not supported yet)
							(void) pack_putc(0xFF, fp);						//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(sp->legacy_track[tracknum]->solo[ctr].start_pos, fp);	//Write the solo's position
							(void) pack_iputl(sp->legacy_track[tracknum]->solo[ctr].end_pos, fp);		//Write the solo's end position
							(void) pack_iputl(0, fp);						//Write section flags (not used)
						}
					}
					if(has_star_power)
					{	//Write star power sections
						(void) pack_iputw(EOF_SP_SECTION, fp);		//Write star power section type
						(void) pack_iputl(sp->legacy_track[tracknum]->star_power_paths, fp);	//Write number of star power sections for this track
						for(ctr=0; ctr < sp->legacy_track[tracknum]->star_power_paths; ctr++)
						{	//For each star power section in the track
							(void) eof_save_song_string_pf(NULL, fp);		//Write an empty section name string (not supported yet)
							(void) pack_putc(0xFF, fp);						//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(sp->legacy_track[tracknum]->star_power_path[ctr].start_pos, fp);	//Write the SP phrase's position
							(void) pack_iputl(sp->legacy_track[tracknum]->star_power_path[ctr].end_pos, fp);	//Write the SP phrase's end position
							(void) pack_iputl(0, fp);						//Write section flags (not used)
						}
					}
					if(has_trills)
					{	//Write trill sections
						(void) pack_iputw(EOF_TRILL_SECTION, fp);		//Write trill section type
						(void) pack_iputl(sp->legacy_track[tracknum]->trills, fp);	//Write number of trill sections for this track
						for(ctr=0; ctr < sp->legacy_track[tracknum]->trills; ctr++)
						{	//For each trill section in the track
							(void) eof_save_song_string_pf(NULL, fp);		//Write an empty section name string (not supported yet)
							(void) pack_putc(0xFF, fp);						//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(sp->legacy_track[tracknum]->trill[ctr].start_pos, fp);	//Write the trill phrase's position
							(void) pack_iputl(sp->legacy_track[tracknum]->trill[ctr].end_pos, fp);		//Write the trill phrase's end position
							(void) pack_iputl(0, fp);						//Write section flags (not used)
						}
					}
					if(has_tremolos)
					{	//Write tremolo sections
						(void) pack_iputw(EOF_TREMOLO_SECTION, fp);		//Write tremolo section type
						(void) pack_iputl(sp->legacy_track[tracknum]->tremolos, fp);	//Write number of tremolo sections for this track
						for(ctr=0; ctr < sp->legacy_track[tracknum]->tremolos; ctr++)
						{	//For each tremolo section in the track
							(void) eof_save_song_string_pf(NULL, fp);		//Write an empty section name string (not supported yet)
							(void) pack_putc(0xFF, fp);						//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(sp->legacy_track[tracknum]->tremolo[ctr].start_pos, fp);	//Write the tremolo phrase's position
							(void) pack_iputl(sp->legacy_track[tracknum]->tremolo[ctr].end_pos, fp);		//Write the tremolo phrase's end position
							(void) pack_iputl(0, fp);						//Write section flags (not used)
						}
					}
					if(has_sliders)
					{	//Write slider sections
						(void) pack_iputw(EOF_SLIDER_SECTION, fp);			//Write slider section type
						(void) pack_iputl(sp->legacy_track[tracknum]->sliders, fp);	//Write number of slider sections for this track
						for(ctr=0; ctr < sp->legacy_track[tracknum]->sliders; ctr++)
						{	//For each slider section in the track
							(void) eof_save_song_string_pf(NULL, fp);		//Write an empty section name string (not supported yet)
							(void) pack_putc(0xFF, fp);						//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(sp->legacy_track[tracknum]->slider[ctr].start_pos, fp);	//Write the slider phrase's position
							(void) pack_iputl(sp->legacy_track[tracknum]->slider[ctr].end_pos, fp);	//Write the slider phrase's end position
							(void) pack_iputl(0, fp);						//Write section flags (not used)
						}
					}
				break;
				case EOF_VOCAL_TRACK_FORMAT:	//Vocal
					(void) pack_putc(0, fp);	//Write the tone set number assigned to this track (not supported yet)
					(void) pack_iputl(sp->vocal_track[tracknum]->lyrics, fp);	//Write the number of lyrics in this track
					for(ctr=0; ctr < sp->vocal_track[tracknum]->lyrics; ctr++)
					{	//For each lyric in this track
						(void) eof_save_song_string_pf(sp->vocal_track[tracknum]->lyric[ctr]->text, fp);	//Write the lyric string
						(void) pack_putc(0, fp);	//Write lyric set number (not supported yet)
						(void) pack_putc(sp->vocal_track[tracknum]->lyric[ctr]->note, fp);		//Write the lyric pitch
						(void) pack_iputl(sp->vocal_track[tracknum]->lyric[ctr]->pos, fp);		//Write the lyric position
						(void) pack_iputl(sp->vocal_track[tracknum]->lyric[ctr]->length, fp);	//Write the lyric length
						(void) pack_iputl(0, fp);	//Write the lyric flags (not supported yet)
					}
					//Write the section type chunk
					if(sp->vocal_track[tracknum]->lines)
					{
						has_lyric_phrases = 1;
					}
					(void) pack_iputw(has_lyric_phrases, fp);	//Write number of section types
					if(has_lyric_phrases)
					{	//Write lyric phrases
						(void) pack_iputw(EOF_LYRIC_PHRASE_SECTION, fp);	//Write lyric phrase section type
						(void) pack_iputl(sp->vocal_track[tracknum]->lines, fp);	//Write number of star power sections for this track
						for(ctr=0; ctr < sp->vocal_track[tracknum]->lines; ctr++)
						{	//For each lyric phrase in the track
							(void) eof_save_song_string_pf(NULL, fp);		//Write an empty section name string (not supported yet)
							(void) pack_putc(0, fp);						//Write the associated difficulty (lyric set) (not supported yet)
							(void) pack_iputl(sp->vocal_track[tracknum]->line[ctr].start_pos, fp);	//Write the lyric phrase's position
							(void) pack_iputl(sp->vocal_track[tracknum]->line[ctr].end_pos, fp);	//Write the lyric phrase's end position
							(void) pack_iputl(sp->vocal_track[tracknum]->line[ctr].flags, fp);		//Write section flags
						}
					}
				break;
				case EOF_PRO_KEYS_TRACK_FORMAT:	//Pro Keys
					allegro_message("Error: Pro Keys not supported yet.  Aborting");
					(void) pack_fclose(fp);
				return 0;
				case EOF_PRO_GUITAR_TRACK_FORMAT:	//Pro Guitar/Bass
				{
					tp = sp->pro_guitar_track[tracknum];	//Simplify
					(void) pack_putc(tp->numfrets, fp);		//Write the number of frets used in this track
					(void) pack_putc(tp->numstrings, fp);	//Write the number of strings used in this track
					for(ctr=0; ctr < tp->numstrings; ctr++)
					{	//For each string
						(void) pack_putc(tp->tuning[ctr], fp);	//Write this string's tuning value
					}
					if(tp->note == tp->technote)
					{	//If tech view is in effect for the track being written
						restore_tech_view = 1;
						eof_menu_pro_guitar_track_disable_tech_view(tp);	//Temporarily change to the regular note array
					}
					(void) pack_iputl(tp->notes, fp);			//Write the number of notes in this track
					for(ctr=0; ctr < tp->notes; ctr++)
					{	//For each note in this track
						eof_write_pro_guitar_note(tp->note[ctr], fp);	//Write the note to the PACKFILE
					}
					//Write the section type chunk, first count the number of section types to write
					if(tp->solos)
					{
						has_solos = 1;
					}
					if(tp->star_power_paths)
					{
						has_star_power = 1;
					}
					if(tp->arpeggios)
					{
						has_arpeggios = 1;
					}
					if(tp->trills)
					{
						has_trills = 1;
					}
					if(tp->tremolos)
					{
						has_tremolos = 1;
					}
					if(tp->handpositions)
					{
						has_handpositions = 1;
					}
					if(tp->popupmessages)
					{
						has_popupmesages = 1;
					}
					if(tp->tonechanges)
					{
						has_tonechanges = 1;
					}
					(void) pack_iputw(has_solos + has_star_power + has_arpeggios + has_trills + has_tremolos + has_handpositions + has_popupmesages + has_tonechanges, fp);		//Write the number of section types
					if(has_solos)
					{	//Write solo sections
						(void) pack_iputw(EOF_SOLO_SECTION, fp);			//Write solo section type
						(void) pack_iputl(tp->solos, fp);					//Write number of solo sections for this track
						for(ctr=0; ctr < tp->solos; ctr++)
						{	//For each solo section in the track
							(void) eof_save_song_string_pf(NULL, fp);		//Write an empty section name string (not supported yet)
							(void) pack_putc(0xFF, fp);						//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(tp->solo[ctr].start_pos, fp);	//Write the solo's position
							(void) pack_iputl(tp->solo[ctr].end_pos, fp);	//Write the solo's end position
							(void) pack_iputl(0, fp);						//Write section flags (not used)
						}
					}
					if(has_star_power)
					{	//Write star power sections
						(void) pack_iputw(EOF_SP_SECTION, fp);				//Write star power section type
						(void) pack_iputl(tp->star_power_paths, fp);		//Write number of star power sections for this track
						for(ctr=0; ctr < tp->star_power_paths; ctr++)
						{	//For each star power section in the track
							(void) eof_save_song_string_pf(NULL, fp);		//Write an empty section name string (not supported yet)
							(void) pack_putc(0xFF, fp);						//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(tp->star_power_path[ctr].start_pos, fp);	//Write the SP phrase's position
							(void) pack_iputl(tp->star_power_path[ctr].end_pos, fp);	//Write the SP phrase's end position
							(void) pack_iputl(0, fp);						//Write section flags (not used)
						}
					}
					if(has_arpeggios)
					{	//Write arpeggio section
						(void) pack_iputw(EOF_ARPEGGIO_SECTION, fp);		//Write arpeggio section type
						(void) pack_iputl(tp->arpeggios, fp);				//Write number of arpeggio sections for this track
						for(ctr=0; ctr < tp->arpeggios; ctr++)
						{	//For each arpegio section in the track
							(void) eof_save_song_string_pf(NULL, fp);			//Write an empty section name string (not supported yet)
							(void) pack_putc(tp->arpeggio[ctr].difficulty, fp);	//Write the arpeggio phrase's associated difficulty
							(void) pack_iputl(tp->arpeggio[ctr].start_pos, fp);	//Write the arpeggio phrase's position
							(void) pack_iputl(tp->arpeggio[ctr].end_pos, fp);	//Write the arpeggio phrase's end position
							(void) pack_iputl(tp->arpeggio[ctr].flags, fp);		//Write section flags
						}
					}
					if(has_trills)
					{	//Write trill sections
						(void) pack_iputw(EOF_TRILL_SECTION, fp);		//Write trill section type
						(void) pack_iputl(tp->trills, fp);				//Write number of trill sections for this track
						for(ctr=0; ctr < tp->trills; ctr++)
						{	//For each trill section in the track
							(void) eof_save_song_string_pf(NULL, fp);			//Write an empty section name string (not supported yet)
							(void) pack_putc(0xFF, fp);							//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(tp->trill[ctr].start_pos, fp);	//Write the trill phrase's position
							(void) pack_iputl(tp->trill[ctr].end_pos, fp);		//Write the trill phrase's end position
							(void) pack_iputl(0, fp);							//Write section flags (not used)
						}
					}
					if(has_tremolos)
					{	//Write tremolo sections
						(void) pack_iputw(EOF_TREMOLO_SECTION, fp);		//Write tremolo section type
						(void) pack_iputl(tp->tremolos, fp);			//Write number of tremolo sections for this track
						for(ctr=0; ctr < tp->tremolos; ctr++)
						{	//For each tremolo section in the track
							(void) eof_save_song_string_pf(NULL, fp);			//Write an empty section name string (not supported yet)
							(void) pack_putc(tp->tremolo[ctr].difficulty, fp);	//Write the tremolo phrase's difficulty
							(void) pack_iputl(tp->tremolo[ctr].start_pos, fp);	//Write the tremolo phrase's position
							(void) pack_iputl(tp->tremolo[ctr].end_pos, fp);	//Write the tremolo phrase's end position
							(void) pack_iputl(0, fp);							//Write section flags (not used)
						}
					}
					if(has_handpositions)
					{	//Write fret hand positions
						(void) pack_iputw(EOF_FRET_HAND_POS_SECTION, fp);		//Write fret hand position section type
						(void) pack_iputl(tp->handpositions, fp);				//Write number of fret hand positions for this track
						for(ctr=0; ctr < tp->handpositions; ctr++)
						{	//For each fret hand position in the track
							(void) eof_save_song_string_pf(NULL, fp);				//Write an empty section name string (not supported yet)
							(void) pack_putc(tp->handposition[ctr].difficulty, fp);	//Write the fret hand position's associated difficulty
							(void) pack_iputl(tp->handposition[ctr].start_pos, fp);	//Write the fret hand position's timestamp
							(void) pack_iputl(tp->handposition[ctr].end_pos, fp);	//Write the fret hand position's fret number
							(void) pack_iputl(0, fp);								//Write section flags (not used)
						}
					}
					if(has_popupmesages)
					{	//Write popup messages
						(void) pack_iputw(EOF_RS_POPUP_MESSAGE, fp);		//Write popup message section type
						(void) pack_iputl(tp->popupmessages, fp);			//Write number of popup messages for this track
						for(ctr=0; ctr < tp->popupmessages; ctr++)
						{	//For each popup message in the track
							(void) eof_save_song_string_pf(tp->popupmessage[ctr].name, fp);		//Write message text
							(void) pack_putc(0xFF, fp);		//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(tp->popupmessage[ctr].start_pos, fp);		//Write the message's start timestamp
							(void) pack_iputl(tp->popupmessage[ctr].end_pos, fp);		//Write the message's end timestamp
							(void) pack_iputl(0, fp);						//Write section flags (not used)
						}
					}
					if(has_tonechanges)
					{	//Write tone changes
						(void) pack_iputw(EOF_RS_TONE_CHANGE, fp);		//Write tone change message section type
						(void) pack_iputl(tp->tonechanges, fp);			//Write number of tone changes for this track
						for(ctr=0; ctr < tp->tonechanges; ctr++)
						{	//For each tone change in the track
							(void) eof_save_song_string_pf(tp->tonechange[ctr].name, fp);		//Write tone name
							(void) pack_putc(0xFF, fp);		//Write an associated difficulty of "all difficulties"
							(void) pack_iputl(tp->tonechange[ctr].start_pos, fp);		//Write the change's start timestamp
							if(!strcmp(tp->tonechange[ctr].name, tp->defaulttone))
							{	//If this tone is the default tone for the track
								(void) pack_iputl(1, fp);	//Write the change's end timestamp to reflect that this change's tone is the default tone
							}
							else
							{
								(void) pack_iputl(0, fp);	//Write the change's end timestamp to reflect that this change's tone is NOT the default tone
							}
							(void) pack_iputl(0, fp);		//Write section flags (not used)
						}
					}
				}
				break;//Pro Guitar/Bass
				case EOF_PRO_VARIABLE_LEGACY_TRACK_FORMAT:	//Variable Lane Legacy (not implemented yet)
					allegro_message("Error: Variable lane not supported yet.  Aborting");
					(void) pack_fclose(fp);
				return 0;
				default://Unknown track type
					allegro_message("Error: Unsupported track type.  Aborting");
					(void) pack_fclose(fp);
				return 0;
			}//Perform the appropriate logic to write this format of track
		}//Write other tracks

		//Write custom track data blocks
		fingerdefinitions = has_fingerdefinitions = has_arrangement = ignore_tuning = has_capo = has_tech_notes = has_accent = 0;
		if(track_ctr && (sp->track[track_ctr]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT))
		{	//If this is a pro guitar track
			//Count the number of notes with finger definitions
			for(ctr = 0; ctr < tp->notes; ctr++)
			{	//For each note in the track
				for(ctr2 = 0, bitmask = 1; ctr2 < 6; ctr2++, bitmask <<= 1)
				{	//For each supported string in the track
					if(tp->note[ctr]->note & bitmask)
					{	//If this string is used
						fingerdefinitions++;	//Increment the counter representing the number of finger usage bytes to write in this block
						has_fingerdefinitions = 1;
					}
				}
			}
			if(tp->arrangement)
			{	//If this track has a Rocksmith arrangement type defined
				has_arrangement = 1;
			}
			ignore_tuning = 1;	//For now, force the ignore tuning setting (on by default) to be written, so the user can explicitly disable it
			if(tp->capo)
			{	//If this track uses a capo
				has_capo = 1;
			}
			if(tp->technotes)
			{	//If this track has any tech notes defined
				has_tech_notes = 1;
			}
		}//If this is a pro guitar track
		if(track_ctr && (sp->track[track_ctr]->track_format == EOF_LEGACY_TRACK_FORMAT))
		{	//If this is a legacy track
			//Check if any notes use accent status
			for(ctr = 0; ctr < sp->legacy_track[tracknum]->notes; ctr++)
			{	//For each note in the track
				if(eof_get_note_accent(sp, track_ctr, ctr))
				{	//If at least one gem in the note is accented
					has_accent = 1;
					break;
				}
			}
		}
		track_custom_block_count = has_fingerdefinitions + has_arrangement + ignore_tuning + has_capo + has_tech_notes + has_accent;
		if(track_custom_block_count)
		{	//If writing data in a custom data block
			(void) pack_iputl(track_custom_block_count, fp);		//Write the number of custom data blocks
			if(track_ctr && (sp->track[track_ctr]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT))
			{	//If this is a pro guitar track
				if(has_fingerdefinitions)
				{	//Write finger definitions
					(void) pack_iputl(fingerdefinitions + 4, fp);	//Write the number of bytes this block will contain (finger data and a 4 byte block ID)
					(void) pack_iputl(2, fp);		//Write the pro guitar finger array custom data block ID
					for(ctr = 0; ctr < tp->notes; ctr++)
					{	//For each note in the track
						for(ctr2 = 0, bitmask = 1; ctr2 < tp->numstrings; ctr2++, bitmask <<= 1)
						{	//For each supported string in the track
							if(tp->note[ctr]->note & bitmask)
							{	//If this string is used
								(void) pack_putc(tp->note[ctr]->finger[ctr2], fp);	//Write this string's finger value
							}
						}
					}
				}
				if(has_arrangement)
				{	//Write track arrangement type
					(void) pack_iputl(5, fp);		//Write the number of bytes this block will contain (1 byte arrangement type and a 4 byte block ID)
					(void) pack_iputl(3, fp);		//Write the pro guitar track arrangement type custom data block ID
					(void) pack_putc(tp->arrangement, fp);	//Write the pro guitar track arrangement type
				}
				if(ignore_tuning)
				{	//Write track tuning not honored
					(void) pack_iputl(5, fp);		//Write the number of bytes this block will contain (1 byte tuning not honored status and a 4 byte block ID)
					(void) pack_iputl(4, fp);		//Write the pro guitar track tuning not honored custom data block ID
					(void) pack_putc(tp->ignore_tuning, fp);	//Write the track tuning not honored boolean option
				}
				if(has_capo)
				{	//Write capo value
					(void) pack_iputl(5, fp);		//Write the number of bytes this block will contain (1 byte capo position and a 4 byte block ID)
					(void) pack_iputl(6, fp);		//Write the pro guitar capo custom data block ID
					(void) pack_putc(tp->capo, fp);	//Write the track's capo position
				}
				if(has_tech_notes)
				{	//Write tech notes
					PACKFILE *tempf;	//Since the size of the custom data block must be known in advance, write it to a temp file so its size can be read
					unsigned long file_size;
					char tempfilename[] = "eof_tech_notes.tmp";

					tempf = pack_fopen(tempfilename, "w");
					if(!tempf)
					{	//If there was an error opening the temp file for writing
						eof_log("Error writing tech notes to temp file, skipping export of the track's tech notes", 1);
						(void) pack_iputl(8, fp);	//Write the number of bytes this block will contain (4 byte number of tech notes and a 4 byte block ID)
						(void) pack_iputl(7, fp);	//Write the pro guitar tech note custom data block ID
						(void) pack_iputl(0, fp);	//Write 0 tech notes
					}
					else
					{	//The temp file was opened for writing
						(void) pack_iputl(tp->technotes, tempf);	//Write the number of tech notes
						for(ctr = 0; ctr < tp->technotes; ctr++)
						{	//For each tech note in this track
							eof_write_pro_guitar_note(tp->technote[ctr], tempf);	//Write the tech note to the temp file
						}
						(void) pack_fclose(tempf);	//Close temp file
						file_size = (unsigned long)file_size_ex(tempfilename);
						(void) pack_iputl(file_size + 4, fp);	//Write the number of bytes this block will contain (the temp file and a 4 byte block ID)
						(void) pack_iputl(7, fp);				//Write the pro guitar tech note custom data block ID
						tempf = pack_fopen(tempfilename, "r");	//Open temp file for reading
						if(!tempf)
						{	//If the temp file couldn't be accessed
							eof_log("Error reading tech notes temp file.  Aborting", 1);
							(void) pack_fclose(fp);
							return 0;
						}
						for(ctr = 0; ctr < file_size; ctr++)
						{	//For each byte in the temp file
							(void) pack_putc(pack_getc(tempf), fp);	//Copy the byte to the project file
						}
						(void) pack_fclose(tempf);			//Close temp file
						(void) delete_file(tempfilename);	//And delete it
					}
				}//Write tech notes
			}//If this is a pro guitar track
			if(track_ctr && (sp->track[track_ctr]->track_format == EOF_LEGACY_TRACK_FORMAT))
			{	//If this is a legacy track
				if(has_accent)
				{	//Write accent note bitmasks
					(void) pack_iputl(sp->legacy_track[tracknum]->notes + 4, fp);	//Write the number of bytes this block will contain (accent bitmask data and a 4 byte block ID)
					(void) pack_iputl(8, fp);		//Write the accent note bitmask custom data block ID
					for(ctr = 0; ctr < sp->legacy_track[tracknum]->notes; ctr++)
					{	//For each note in the track
						(void) pack_putc(eof_get_note_accent(sp, track_ctr, ctr), fp);	//Write this note's accent bitmask
					}
				}
			}
		}
		else
		{	//Otherwise write a debug custom data block
			(void) pack_iputl(1, fp);			//Write one custom data block
			(void) pack_iputl(4, fp);
			(void) pack_iputl(0xFFFFFFFF, fp);	//Write the debug custom data block ID
		}
		if(tp && restore_tech_view)
		{	//If tech view needs to be re-enabled for the track that was just written
			eof_menu_pro_guitar_track_enable_tech_view(tp);
		}
	}//For each track in the project

	(void) pack_fclose(fp);
	sp->tags->unshare_drum_phrasing = unshare_drum_phrasing;	//After all tracks have been formally written, store this value back into the project to optionally override drum phrase handling
	eof_log("\tSave completed", 1);
	return 1;	//Return success
}

EOF_SONG * eof_create_song_populated(void)
{
	EOF_SONG * sp;
	unsigned long ctr;

 	eof_log("eof_create_song_populated() entered", 1);

	/* create empty song */
	sp = eof_create_song();
	if(sp != NULL)
	{
		for(ctr = 1; ctr < EOF_TRACKS_MAX; ctr++)
		{	//For each track in the eof_default_tracks[] array
			if(eof_song_add_track(sp,&eof_default_tracks[ctr]) == 0)
			{
				eof_destroy_song(sp);	//Destroy the song and return on error
				return NULL;
			}
		}
	}

	return sp;
}

unsigned long eof_count_track_lanes(EOF_SONG *sp, unsigned long track)
{
// 	eof_log("eof_count_track_lanes() entered");

	if((sp == NULL) || (track == 0) || (track >= sp->tracks))
		return 5;	//Return default value if the specified track doesn't exist

	if(sp->track[track]->track_format == EOF_LEGACY_TRACK_FORMAT)
	{	//If this is a legacy track, return the number of lanes it uses
		if(sp->legacy_track[sp->track[track]->tracknum]->numlanes <= EOF_MAX_FRETS)
		{	//If the lane count is valid
			return sp->legacy_track[sp->track[track]->tracknum]->numlanes;
		}
	}
	else if(sp->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
	{
		if(eof_legacy_view)
		{	//If the legacy view is in effect, force pro guitar tracks to render as 5 lane tracks
			return 5;
		}
		else
		{
			if(sp->pro_guitar_track[sp->track[track]->tracknum]->numstrings <= EOF_MAX_FRETS)
			{	//If the string count is valid
				return sp->pro_guitar_track[sp->track[track]->tracknum]->numstrings;
			}
		}
	}

	//Otherwise return 5, as so far, other track formats don't store this information, or an invalid lane count is in effect
	return 5;
}

int eof_open_strum_enabled(unsigned long track)
{
	if(!eof_song || (track >= eof_song->tracks))
		return 0;

	if((track == EOF_TRACK_DRUM) || (track == EOF_TRACK_DRUM_PS))
	{	//Drum tracks do not use open strumming
		return 0;
	}
	return (eof_song->track[track]->flags & EOF_TRACK_FLAG_SIX_LANES);
}

EOF_PRO_GUITAR_NOTE *eof_pro_guitar_track_add_note(EOF_PRO_GUITAR_TRACK *tp)
{
	if(tp && (tp->notes < EOF_MAX_NOTES))
	{
		tp->note[tp->notes] = malloc(sizeof(EOF_PRO_GUITAR_NOTE));
		if(tp->note[tp->notes])
		{
			memset(tp->note[tp->notes], 0, sizeof(EOF_PRO_GUITAR_NOTE));
			tp->notes++;
			return tp->note[tp->notes - 1];
		}
	}
	return NULL;
}

EOF_PRO_GUITAR_NOTE *eof_pro_guitar_track_add_tech_note(EOF_PRO_GUITAR_TRACK *tp)
{
	if(tp && (tp->technotes < EOF_MAX_NOTES))
	{
		tp->technote[tp->technotes] = malloc(sizeof(EOF_PRO_GUITAR_NOTE));
		if(tp->technote[tp->technotes])
		{
			memset(tp->technote[tp->technotes], 0, sizeof(EOF_PRO_GUITAR_NOTE));
			tp->technotes++;
			return tp->technote[tp->technotes - 1];
		}
	}
	return NULL;
}

unsigned long eof_get_track_size(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks) || (sp->track[track] == NULL))
		return 0;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
		return sp->legacy_track[tracknum]->notes;

		case EOF_VOCAL_TRACK_FORMAT:
		return sp->vocal_track[tracknum]->lyrics;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return sp->pro_guitar_track[tracknum]->notes;
	}

	return 0;
}

unsigned long eof_get_track_tech_note_size(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks) || (sp->track[track] == NULL))
		return 0;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
		return 0;

		case EOF_VOCAL_TRACK_FORMAT:
		return 0;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return sp->pro_guitar_track[tracknum]->technotes;
	}

	return 0;
}

unsigned long eof_get_chart_size(EOF_SONG *sp)
{
	unsigned long notectr = 0, trackctr;

	if(sp == NULL)
		return 0;

	for(trackctr = 1; trackctr < sp->tracks; trackctr++)
	{
		notectr += eof_get_track_size(sp, trackctr);	//Add the number of notes in this track to the counter
	}

	return notectr;
}

void *eof_track_add_note(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

 	eof_log("eof_track_add_note() entered", 2);	//Only log this if verbose logging is on

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
		return eof_legacy_track_add_note(sp->legacy_track[tracknum]);

		case EOF_VOCAL_TRACK_FORMAT:
		return eof_vocal_track_add_lyric(sp->vocal_track[tracknum]);

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return eof_pro_guitar_track_add_note(sp->pro_guitar_track[tracknum]);
	}

	return NULL;
}

void eof_track_delete_note(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

 	eof_log("eof_track_delete_note() entered", 2);	//Only log this if verbose logging is on

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	if(note < eof_get_track_size(sp, track))
	{
		switch(sp->track[track]->track_format)
		{
			case EOF_LEGACY_TRACK_FORMAT:
				eof_legacy_track_delete_note(sp->legacy_track[tracknum], note);
			break;

			case EOF_VOCAL_TRACK_FORMAT:
				eof_vocal_track_delete_lyric(sp->vocal_track[tracknum], note);
			break;

			case EOF_PRO_GUITAR_TRACK_FORMAT:
				eof_pro_guitar_track_delete_note(sp->pro_guitar_track[tracknum], note);
			break;
		}
	}
}

void eof_track_delete_note_with_difficulties(EOF_SONG *sp, unsigned long track, unsigned long notenum, int operation)
{
	unsigned long ctr, notepos;

	if(!sp || (track >= sp->tracks))
		return;	//Invalid parameters

	//Delete notes in higher difficulties if applicable
	notepos = eof_get_note_pos(sp, track, notenum);	//Cache this value
	if(operation > 0)
	{	//If the calling function wanted to delete ALL notes at the specified note's position
		for(ctr = eof_get_track_size(sp, track); ctr > notenum + 1; ctr--)
		{	//For each note in the track AFTER the specified one, in reverse order
			if((notepos == eof_get_note_pos(sp, track, ctr - 1)) && (eof_get_note_type(sp, track, ctr - 1) > eof_note_type))
			{	//If this note is at the same position and in a higher difficulty
				eof_track_delete_note(sp, track, ctr - 1);
			}
		}
	}

	//Delete the note in the active difficulty
	eof_track_delete_note(sp, track, notenum);
	eof_selection.multi[notenum] = 0;

	//Delete notes in lower difficulties if applicable
	if(operation)
	{	//If the calling function wanted to delete notes at the specified note's position in all difficulties or in lower difficulties
		for(ctr = notenum; ctr > 0; ctr--)
		{	//For each note in the track BEFORE the specified one, in reverse order
			if((notepos == eof_get_note_pos(sp, track, ctr - 1)) && (eof_get_note_type(sp, track, ctr - 1) < eof_note_type))
			{	//If this note is at the same position and in a lower difficulty
				eof_track_delete_note(eof_song, eof_selected_track, ctr - 1);
			}
		}
	}
}

void eof_song_empty_track(EOF_SONG * sp, unsigned long track)
{
	unsigned long i;

	if((sp == NULL) || (track >= sp->tracks))
		return;

	if(sp->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
	{	//If the track being destroyed is a pro guitar track, erase the technote array first
		EOF_PRO_GUITAR_TRACK *tp = sp->pro_guitar_track[sp->track[track]->tracknum];

		eof_menu_pro_guitar_track_enable_tech_view(tp);	//Empty the tech note array first
		for(i = eof_get_track_size(sp, track); i > 0; i--)
		{	//Delete all notes in reverse order, which will avoid having to re-arrange the note array after each
			eof_track_delete_note(sp, track, i-1);	//Note numbering starts with #0
		}
		eof_menu_pro_guitar_track_disable_tech_view(tp);	//Then empty the regular note array
	}

	for(i = eof_get_track_size(sp, track); i > 0; i--)
	{	//Delete all notes in reverse order, which will avoid having to re-arrange the note array after each
		eof_track_delete_note(sp, track, i-1);	//Note numbering starts with #0
	}
}

void eof_track_resize(EOF_SONG *sp, unsigned long track, unsigned long size)
{
	unsigned long i, oldsize;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	oldsize = eof_get_track_size(sp, track);

	if(size > oldsize)
	{	//If this track is being grown
		for(i=oldsize; i < size; i++)
		{
			(void) eof_track_add_note(sp, track);
		}
	}
	else if(size < oldsize)
	{	//If this track is being shrunk
		for(i=oldsize; i > size; i--)
		{	//Delete notes from the end of the array
			eof_track_delete_note(sp, track, i-1);
		}
	}
}

unsigned char eof_get_note_type(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0xFF;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				return sp->legacy_track[tracknum]->note[note]->type;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				return sp->vocal_track[tracknum]->lyric[note]->type;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				return sp->pro_guitar_track[tracknum]->note[note]->type;
			}
		break;
	}

	return 0xFF;	//Return error
}

unsigned long eof_get_note_pos(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				return sp->legacy_track[tracknum]->note[note]->pos;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				return sp->vocal_track[tracknum]->lyric[note]->pos;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				return sp->pro_guitar_track[tracknum]->note[note]->pos;
			}
		break;
	}

	return 0;	//Return error
}

long eof_get_note_length(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				return sp->legacy_track[tracknum]->note[note]->length;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				return sp->vocal_track[tracknum]->lyric[note]->length;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				return sp->pro_guitar_track[tracknum]->note[note]->length;
			}
		break;
	}

	return 0;	//Return error
}

void eof_set_note_length(EOF_SONG *sp, unsigned long track, unsigned long note, long length)
{
// 	eof_log("eof_set_note_length() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				sp->legacy_track[tracknum]->note[note]->length = length;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				sp->vocal_track[tracknum]->lyric[note]->length = length;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				sp->pro_guitar_track[tracknum]->note[note]->length = length;
			}
		break;
	}
}

unsigned long eof_get_note_flags(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				return sp->legacy_track[tracknum]->note[note]->flags;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				return sp->vocal_track[tracknum]->lyric[note]->flags;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				return sp->pro_guitar_track[tracknum]->note[note]->flags;
			}
		break;
	}

	return 0;	//Return error
}

unsigned short eof_get_note_tflags(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				return sp->legacy_track[tracknum]->note[note]->tflags;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				return sp->vocal_track[tracknum]->lyric[note]->tflags;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				return sp->pro_guitar_track[tracknum]->note[note]->tflags;
			}
		break;
	}

	return 0;	//Return error
}

unsigned char eof_get_note_eflags(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
		return 0;	//Legacy tracks do not use extended flags yet

		case EOF_VOCAL_TRACK_FORMAT:
		return 0;	//Vocal tracks do not use extended flags yet

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				return sp->pro_guitar_track[tracknum]->note[note]->eflags;
			}
		break;
	}

	return 0;	//Return error
}

unsigned char eof_get_note_note(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				return sp->legacy_track[tracknum]->note[note]->note;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				return sp->vocal_track[tracknum]->lyric[note]->note;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				return sp->pro_guitar_track[tracknum]->note[note]->note;
			}
		break;
	}

	return 0;	//Return error
}

unsigned char eof_get_note_ghost(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				return sp->pro_guitar_track[tracknum]->note[note]->ghost;
			}
		break;
	}

	return 0;	//Return error or not applicable
}

unsigned char eof_get_note_accent(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks) || !track)
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				return sp->legacy_track[tracknum]->note[note]->accent;
			}
		break;
	}

	return 0;	//Return error
}

unsigned long eof_get_pro_guitar_note_note(EOF_PRO_GUITAR_TRACK *tp, unsigned long note)
{
	if((tp == NULL) || (note >= tp->notes))
		return 0;	//Return error

	return tp->note[note]->note;
}

void *eof_track_add_create_note(EOF_SONG *sp, unsigned long track, unsigned char note, unsigned long pos, long length, char type, char *text)
{
	void *new_note = NULL;
	EOF_NOTE *ptr = NULL;
	EOF_LYRIC *ptr2 = NULL;
	EOF_PRO_GUITAR_NOTE *ptr3 = NULL;

 	eof_log("eof_track_add_create_note() entered", 2);

	if(!sp || (track >= sp->tracks))
	{
		return NULL;
	}

	new_note = eof_track_add_note(sp, track);
	if(new_note != NULL)
	{
		if(length < 1)
		{
			length = 1;	//1 is the minimum length of any note/lyric
		}
		switch(sp->track[track]->track_format)
		{
			case EOF_LEGACY_TRACK_FORMAT:
				ptr = (EOF_NOTE *)new_note;
				ptr->type = type;
				ptr->note = note;
				ptr->midi_pos = 0;	//Not implemented yet
				ptr->midi_length = 0;	//Not implemented yet
				ptr->pos = pos;
				ptr->length = length;
				ptr->flags = 0;
				ptr->tflags = 0;
				if(text != NULL)
				{
					(void) ustrcpy(ptr->name, text);
				}
				else
				{
					ptr->name[0] = '\0';	//Empty the string
				}
				if(!((eof_count_track_lanes(sp, track) > 5) && (track != EOF_TRACK_BASS)))
				{	//If the track storing the new note does not have six lanes (with the exclusion of the bass track's open strum lane)
					note &= ~32;	//Clear lane 6
				}
				if(sp->track[track]->track_behavior == EOF_KEYS_TRACK_BEHAVIOR)
				{	//In a keys track, all lanes are forced to be "crazy" and be allowed to overlap other lanes
					ptr->flags |= EOF_NOTE_FLAG_CRAZY;	//Set the crazy flag bit
				}
			return ptr;

			case EOF_VOCAL_TRACK_FORMAT:
				ptr2 = (EOF_LYRIC *)new_note;
				ptr2->type = type;
				ptr2->note = note;
				if(text != NULL)
				{
					(void) ustrcpy(ptr2->text, text);
				}
				else
				{
					ptr2->text[0] = '\0';	//Empty the string
				}
				ptr2->midi_pos = 0;	//Not implemented yet
				ptr2->midi_length = 0;	//Not implemented yet
				ptr2->pos = pos;
				ptr2->length = length;
				ptr2->flags = 0;
				ptr2->tflags = 0;
			return ptr2;

			case EOF_PRO_GUITAR_TRACK_FORMAT:
				ptr3 = (EOF_PRO_GUITAR_NOTE *)new_note;
				if(text != NULL)
				{
					(void) ustrcpy(ptr3->name, text);
				}
				else
				{
					ptr3->name[0] = '\0';	//Empty the string
				}
				ptr3->type = type;
				ptr3->note = note;
				ptr3->ghost = 0;
				memset(ptr3->frets, 0, 8);	//Initialize all frets to open
				memset(ptr3->finger, 0, 8);	//Initialize all fingers to undefined
				ptr3->midi_pos = 0;			//Not implemented yet
				ptr3->midi_length = 0;		//Not implemented yet
				ptr3->pos = pos;
				ptr3->length = length;
				ptr3->flags = 0;
				ptr3->eflags = 0;
				ptr3->tflags = 0;
				ptr3->bendstrength = 0;
				ptr3->slideend = 0;
				ptr3->unpitchend = 0;
				if(eof_legacy_view)
				{	//If legacy view is in effect, initialize the legacy bitmask to whichever lanes (1-5) created the note
					ptr3->legacymask = note & 31;
				}
				else
				{	//Otherwise initialize the legacy bitmask to zero
					ptr3->legacymask = 0;
				}
			return ptr3;
		}
	}

	return NULL;	//Return error
}

unsigned long eof_get_num_solos(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
		return sp->legacy_track[tracknum]->solos;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return sp->pro_guitar_track[tracknum]->solos;
	}

	return 0;	//Return error
}

EOF_PHRASE_SECTION *eof_get_solo(EOF_SONG *sp, unsigned long track, unsigned long solonum)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(solonum < EOF_MAX_PHRASES)
			{
				if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
				{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
					tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
				}
				return &sp->legacy_track[tracknum]->solo[solonum];
			}
		break;


		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(solonum < EOF_MAX_PHRASES)
			{
				return &sp->pro_guitar_track[tracknum]->solo[solonum];
			}
		break;
	}

	return NULL;	//Return error
}

void eof_set_note_pos(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned long pos)
{
// 	eof_log("eof_set_note_pos() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				sp->legacy_track[tracknum]->note[note]->pos = pos;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				sp->vocal_track[tracknum]->lyric[note]->pos = pos;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				sp->pro_guitar_track[tracknum]->note[note]->pos = pos;
			}
		break;
	}
}

void eof_move_note_pos(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned long amount, char dir)
{
// 	eof_log("eof_move_note_pos() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				if(dir > 0)
				{	//If moving a note forward
					sp->legacy_track[tracknum]->note[note]->pos += amount;
				}
				else if(sp->legacy_track[tracknum]->note[note]->pos - sp->beat[0]->pos >= amount)
				{	//Only move the note earlier if it won't go before the first beat marker
					sp->legacy_track[tracknum]->note[note]->pos -= amount;
				}
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				if(dir > 0)
				{	//If moving a note forward
					sp->vocal_track[tracknum]->lyric[note]->pos += amount;
				}
				else if(sp->vocal_track[tracknum]->lyric[note]->pos - sp->beat[0]->pos >= amount)
				{	//Only move the note earlier if it won't go before the first beat marker
					sp->vocal_track[tracknum]->lyric[note]->pos -= amount;
				}
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				if(dir > 0)
				{	//If moving a note forward
					sp->pro_guitar_track[tracknum]->note[note]->pos += amount;
				}
				else if(sp->pro_guitar_track[tracknum]->note[note]->pos - sp->beat[0]->pos >= amount)
				{	//Only move the note earlier if it won't go before the first beat marker
					sp->pro_guitar_track[tracknum]->note[note]->pos -= amount;
				}
			}
		break;
	}
}

void eof_set_note_note(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned char value)
{
// 	eof_log("eof_set_note_note() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				sp->legacy_track[tracknum]->note[note]->note = value;
				sp->legacy_track[tracknum]->note[note]->accent &= value;	//Clear all accent bits that don't have a corresponding gem that exists
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				sp->vocal_track[tracknum]->lyric[note]->note = value;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				sp->pro_guitar_track[tracknum]->note[note]->note = value;
				memset(sp->pro_guitar_track[tracknum]->note[note]->finger, 0, 8);	//Initialize all fingers to undefined
			}
		break;
	}
}

void eof_set_note_accent(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned char value)
{
// 	eof_log("eof_set_note_note() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				sp->legacy_track[tracknum]->note[note]->accent = value;
			}
		break;
	}
}

void eof_track_sort_notes(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum, tflags, ctr;

 	eof_log("eof_track_sort_notes() entered", 2);

	if((sp == NULL) || (track >= sp->tracks) || (track == 0))
		return;
	tracknum = sp->track[track]->tracknum;

	//Preserve the note selection, which would be destroyed by note sorting, if applicable
	if(eof_selection.track == track)
	{	//If the track being sorted has selected notes
		for(ctr = eof_get_track_size(sp, track); ctr > 0; ctr--)
		{	//For each note in the track, in reverse order
			if(eof_selection.multi[ctr - 1])
			{	//If this note is selected
				tflags = eof_get_note_tflags(sp, track, ctr - 1);
				tflags |= EOF_NOTE_TFLAG_SORT;	//Set this temporary flag to track this note is selected
				eof_set_note_tflags(sp, track, ctr - 1, tflags);	//Update the note flags
			}
		}
	}

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			eof_legacy_track_sort_notes(sp->legacy_track[tracknum]);
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			eof_vocal_track_sort_lyrics(sp->vocal_track[tracknum]);
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			eof_pro_guitar_track_sort_notes(sp->pro_guitar_track[tracknum]);
		break;
	}

	//Recreate the note selection if applicable
	if(eof_selection.track == track)
	{	//If the track being sorted has selected notes
		for(ctr = eof_get_track_size(sp, track); ctr > 0; ctr--)
		{	//For each note in the track, in reverse order
			tflags = eof_get_note_tflags(sp, track, ctr - 1);
			if(tflags & EOF_NOTE_TFLAG_SORT)
			{	//If this note was previously marked as selected
				eof_selection.multi[ctr - 1] = 1;	//Mark this note as selected in the selection array
			}
			else
			{	//This note was not selected before the sort was performed
				eof_selection.multi[ctr - 1] = 0;
			}
			tflags &= ~EOF_NOTE_TFLAG_SORT;	//Clear the temporary flag
			eof_set_note_tflags(sp, track, ctr - 1, tflags);	//Restore the note's original flags
		}
	}
}

void eof_track_fixup_notes(EOF_SONG *sp, unsigned long track, int sel)
{
 	eof_log("eof_track_fixup_notes() entered", 2);

	if((sp == NULL) || (track >= sp->tracks))
		return;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			eof_legacy_track_fixup_notes(sp, track, sel);
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			eof_vocal_track_fixup_lyrics(sp, track, sel);
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			eof_pro_guitar_track_fixup_notes(sp, track, sel);
		break;
	}

	if(sp->tags->highlight_unsnapped_notes || sp->tags->highlight_arpeggios)
	{	//If the user has enabled the dynamic highlighting of non grid snapped notes or notes in arpeggios
		eof_track_remove_highlighting(sp, track, 1);	//Remove existing temporary highlighting from the track
		if(sp->tags->highlight_unsnapped_notes)
			eof_song_highlight_non_grid_snapped_notes(sp, track);	//Re-create the non grid snapped highlighting as appropriate
		if(sp->tags->highlight_arpeggios)
			eof_song_highlight_arpeggios(sp, track);	//Re-create the arpeggio highlighting as appropriate
	}
}

void eof_pro_guitar_track_sort_notes(EOF_PRO_GUITAR_TRACK * tp)
{
	if(tp)
	{
		qsort(tp->note, (size_t)tp->notes, sizeof(EOF_PRO_GUITAR_NOTE *), eof_song_qsort_pro_guitar_notes);
	}
}

int eof_song_qsort_phrases_diff_timestamp(const void * e1, const void * e2)
{
	EOF_PHRASE_SECTION * thing1 = (EOF_PHRASE_SECTION *)e1;
	EOF_PHRASE_SECTION * thing2 = (EOF_PHRASE_SECTION *)e2;

	//Sort by difficulty first
	if(thing1->difficulty < thing2->difficulty)
	{
		return -1;
	}
	else if(thing1->difficulty > thing2->difficulty)
	{
		return 1;
	}

	//Sort by timestamp second
	if(thing1->start_pos < thing2->start_pos)
	{
		return -1;
	}
	else if(thing1->start_pos > thing2->start_pos)
	{
		return 1;
	}

	//They are equal
	return 0;
}

void eof_pro_guitar_track_sort_fret_hand_positions(EOF_PRO_GUITAR_TRACK* tp)
{
 	eof_log("eof_pro_guitar_track_sort_fret_hand_positions() entered", 2);

	if(tp)
	{
		qsort(tp->handposition, (size_t)tp->handpositions, sizeof(EOF_PHRASE_SECTION), eof_song_qsort_phrases_diff_timestamp);
	}
}

void eof_pro_guitar_track_sort_arpeggios(EOF_PRO_GUITAR_TRACK* tp)
{
 	eof_log("eof_pro_guitar_track_sort_arpeggios() entered", 2);

	if(tp)
	{
		qsort(tp->arpeggio, (size_t)tp->arpeggios, sizeof(EOF_PHRASE_SECTION), eof_song_qsort_phrases_diff_timestamp);
	}
}

void eof_pro_guitar_track_delete_hand_position(EOF_PRO_GUITAR_TRACK *tp, unsigned long index)
{
	unsigned long ctr;
 	eof_log("eof_pro_guitar_track_delete_hand_position() entered", 2);

	if(tp && (index < tp->handpositions))
	{
		tp->handposition[index].name[0] = '\0';	//Empty the name string
		for(ctr = index; ctr < tp->handpositions; ctr++)
		{
			memcpy(&tp->handposition[ctr], &tp->handposition[ctr + 1], sizeof(EOF_PHRASE_SECTION));
		}
		tp->handpositions--;
	}
}

/* sort all notes according to position */
int eof_song_qsort_pro_guitar_notes(const void * e1, const void * e2)
{
	EOF_PRO_GUITAR_NOTE ** thing1 = (EOF_PRO_GUITAR_NOTE **)e1;
	EOF_PRO_GUITAR_NOTE ** thing2 = (EOF_PRO_GUITAR_NOTE **)e2;

	//Sort first by chronological order
	if((*thing1)->pos < (*thing2)->pos)
	{
		return -1;
	}
	if((*thing1)->pos > (*thing2)->pos)
	{
		return 1;
	}
	//Sort second by difficulty
	if((*thing1)->type < (*thing2)->type)
	{
		return -1;
	}
	if((*thing1)->type > (*thing2)->type)
	{
		return 1;
	}
	//Sort third by note mask
	if((*thing1)->note < (*thing2)->note)
	{
		return -1;
	}
	if((*thing1)->note > (*thing2)->note)
	{
		return 1;
	}

	// they are equal...
	return 0;
}

int eof_song_qsort_phrase_sections(const void * e1, const void * e2)
{
	EOF_PHRASE_SECTION * thing1 = (EOF_PHRASE_SECTION *)e1;
	EOF_PHRASE_SECTION * thing2 = (EOF_PHRASE_SECTION *)e2;

	//Sort by timestamp
	if(thing1->start_pos < thing2->start_pos)
	{
		return -1;
	}
	else if(thing1->start_pos > thing2->start_pos)
	{
		return 1;
	}

	//They are equal
	return 0;
}

void eof_pro_guitar_track_delete_note(EOF_PRO_GUITAR_TRACK * tp, unsigned long note)
{
	unsigned long i;

	if(tp && (note < tp->notes))
	{
		free(tp->note[note]);
		for(i = note; i < tp->notes - 1; i++)
		{
			tp->note[i] = tp->note[i + 1];
		}
		tp->notes--;
	}
}

long eof_fixup_previous_pro_guitar_note(EOF_PRO_GUITAR_TRACK * tp, unsigned long note)
{
	long i;

	if(!tp || (note >= tp->notes))
		return -1;	//Invalid parameters

	if(tp)
	{
		for(i = note; i > 0; i--)
		{
			if(tp->note[i - 1]->type == tp->note[note]->type)
			{
				return i - 1;
			}
		}
	}
	return -1;
}

long eof_fixup_next_pro_guitar_note(EOF_PRO_GUITAR_TRACK * tp, unsigned long note)
{
	long i;

	if(!tp || (note >= tp->notes))
		return -1;	//Invalid parameters

	if(tp)
	{
		for(i = note + 1; i < tp->notes; i++)
		{
			if(tp->note[i]->type == tp->note[note]->type)
			{
				return i;
			}
		}
	}
	return -1;
}

long eof_track_fixup_first_pro_guitar_note(EOF_PRO_GUITAR_TRACK * tp, unsigned char diff)
{
	unsigned long ctr;

	if(!tp || !tp->parent || (diff > tp->parent->numdiffs))
		return -1;

	for(ctr = 0; ctr < tp->notes; ctr++)
	{	//For each normal note in the track
		if(tp->note[ctr]->type == diff)
		{	//If the note is in the target difficulty
			return ctr;	//Return its index
		}
	}
	return -1;	//Note was not found
}

long eof_get_prev_note_type_num(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	long i;

	if(sp)
	{
		for(i = note - 1; i >= 0; i--)
		{	//For each note before the specified note number, starting with the one immediately before it
			if(eof_get_note_type(sp, track, note) == eof_get_note_type(sp, track, i))
			{	//If this note has the same type/difficulty
				return i;
			}
		}
	}
	return -1;	//Return note not found
}

void eof_pro_guitar_track_fixup_hand_positions(EOF_SONG *sp, unsigned long track)
{
	unsigned ctr, ctr2;
	EOF_PRO_GUITAR_TRACK * tp;

	if(!sp || (track >= sp->tracks) || (sp->track[track]->track_format != EOF_PRO_GUITAR_TRACK_FORMAT))
	{
		return;	//Invalid parameters
	}
	tp = sp->pro_guitar_track[sp->track[track]->tracknum];

	//Cleanup fret hand positions
	eof_pro_guitar_track_sort_fret_hand_positions(tp);	//Ensure fret hand positions are sorted
	for(ctr = tp->handpositions; ctr > 0; ctr--)
	{	//For each fret hand position in the track, in reverse order
		for(ctr2 = ctr; ctr2 < tp->handpositions; ctr2++)
		{	//For each of the following fret hand positions in the track
			if(tp->handposition[ctr - 1].start_pos != tp->handposition[ctr2].start_pos)
			{	//If this fret hand position (and all subsequent ones) are at a different timestamp
				break;	//Exit inner loop
			}
			if(tp->handposition[ctr - 1].difficulty == tp->handposition[ctr2].difficulty)
			{	//If the two hand positions at the same timestamp and track difficulty
				eof_pro_guitar_track_delete_hand_position(tp, ctr2);	//Delete the latter
			}
		}
		if(tp->handposition[ctr - 1].start_pos > sp->beat[sp->beats - 1]->pos)
		{	//If this fret hand position is after the last beat marker
			eof_pro_guitar_track_delete_hand_position(tp, ctr - 1);	//Delete it
		}
		else if((tp->handposition[ctr - 1].end_pos == 0) || (tp->handposition[ctr - 1].end_pos > 21))
		{	//If this fret hand position is at an invalid fret value
			eof_pro_guitar_track_delete_hand_position(tp, ctr - 1);	//Delete it
		}
	}
}

void eof_pro_guitar_track_fixup_notes(EOF_SONG *sp, unsigned long track, int sel)
{
	unsigned long i, ctr, ctr2, ctr3, bitmask, tracknum, maxlength, flags;
	long nextnote;
	unsigned char fretvalue;
	long next;
	char allmuted;	//Used to track whether all used strings are string muted
	EOF_PRO_GUITAR_TRACK * tp;
	EOF_RS_TECHNIQUES ptr = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0};
	char has_link_next;	//Is set to nonzero if any strings used in the note have sustain status applied
	EOF_PHRASE_SECTION *pp, *ppp;
	EOF_PRO_GUITAR_NOTE *np;

	if(!sp || (track >= sp->tracks) || (sp->track[track]->track_format != EOF_PRO_GUITAR_TRACK_FORMAT))
	{
		return;	//Invalid parameters
	}
	tracknum = sp->track[track]->tracknum;
	tp = sp->pro_guitar_track[tracknum];
	if(!sel)
	{
		if(eof_selection.current < tp->notes)
		{
			eof_selection.multi[eof_selection.current] = 0;
		}
		eof_selection.current = EOF_MAX_NOTES - 1;
	}
	//Loop through notes looking for any marked as temporary or ignored and return from function if any are found to avoid merging/deleting them
	for(i = 0; i < tp->notes; i++)
	{	//For each note in the track
		if(tp->note[i]->tflags & (EOF_NOTE_TFLAG_TEMP | EOF_NOTE_TFLAG_IGNORE))
		{	//If this note has temporary or ignore flags
			eof_log("eof_pro_guitar_track_fixup_notes() aborted due to presence of notes with temp/ignore flags", 1);
			eof_log("Rocksmith 2 export is presumably in progress", 1);
			return;
		}
	}
	//Check for overlapping arpeggio phrases and merge them
	for(ctr = tp->arpeggios; ctr > 1; ctr--)
	{	//For each arpeggio phrase, in reverse order from the last arpeggio to the second one
		pp = &tp->arpeggio[ctr - 1];	//Get a pointer to this arpeggio
		for(ctr2 = ctr - 1; ctr2 > 0; ctr2--)
		{	//For each arpeggio phrase that precedes the one in the outer for loop
			ppp = &tp->arpeggio[ctr2 - 1];	//Get a pointer to this preceding arpeggio

			if((pp->start_pos <= ppp->end_pos) && (pp->end_pos >= ppp->start_pos) && (pp->difficulty == ppp->difficulty))
			{	//If this arpeggio overlaps a preceding one and they are in the same track difficulty
				if(pp->end_pos > ppp->end_pos)
				{	//Keep the later end point of the two phrases
					ppp->end_pos = pp->end_pos;
				}
				if(pp->start_pos < ppp->start_pos)
				{	//Keep the earlier start point of the two phrases
					ppp->start_pos = pp->start_pos;
				}
				eof_pro_guitar_track_delete_arpeggio(tp, ctr - 1);	//Delete the arpeggio phrase
				break;	//Break from inner for loop
			}
		}
	}
	for(i = tp->notes; i > 0; i--)
	{	//For each note in the track, in reverse order
		/* ensure notes within an arpeggio phrase (but not a handshape phrase) are marked as "crazy" */
		for(ctr = 0; ctr < tp->arpeggios; ctr++)
		{	//For each arpeggio section in this track
			if((tp->note[i-1]->pos >= tp->arpeggio[ctr].start_pos) && (tp->note[i-1]->pos <= tp->arpeggio[ctr].end_pos) && (tp->note[i-1]->type == tp->arpeggio[ctr].difficulty))
			{	//If this note is in an arpeggio phrase and in the phrase's associated difficulty
				if(!(tp->arpeggio[ctr].flags & EOF_RS_ARP_HANDSHAPE))
				{	//If this is explicitly an arpeggio phrase and NOT a handshape phrase
					tp->note[i-1]->flags |= EOF_NOTE_FLAG_CRAZY;	//Set the crazy flag bit
					break;
				}
			}
		}

		/* fix selections */
		if((tp->note[i-1]->type == eof_note_type) && (tp->note[i-1]->pos == eof_selection.current_pos))
		{
			eof_selection.current = i-1;
		}
		if((tp->note[i-1]->type == eof_note_type) && (tp->note[i-1]->pos == eof_selection.last_pos))
		{
			eof_selection.last = i-1;
		}

		for(ctr=0, bitmask=1; ctr < 8; ctr++, bitmask <<= 1)
		{	//For each lane
			if((tp->note[i-1]->note & bitmask) && (ctr >= tp->numstrings))
			{	//If this lane is populated and is above the highest valid string number for this track
				tp->note[i-1]->note &= (~bitmask);	//Clear the lane
			}
		}

		/* delete invalid notes or force corrections on valid notes */
		if((tp->note[i-1]->note == 0) || (tp->note[i-1]->type >= sp->track[track]->numdiffs) || (tp->note[i-1]->pos < sp->tags->ogg[eof_selected_ogg].midi_offset) || (tp->note[i-1]->pos >= eof_chart_length))
		{	//If the note is not valid
			eof_pro_guitar_track_delete_note(tp, i-1);
		}
		else
		{	//If the note is valid, perform other cleanup
			/* make sure there are no 0-length notes */
			if(tp->note[i-1]->length <= 0)
			{
				tp->note[i-1]->length = 1;
			}

			/* make sure note doesn't extend past end of song */
			if(tp->note[i-1]->pos + tp->note[i-1]->length >= eof_chart_length)
			{
				tp->note[i-1]->length = eof_chart_length - tp->note[i-1]->pos;
			}

			/* compare this note to the next one of the same type
			   to make sure they don't overlap */
			next = eof_fixup_next_pro_guitar_note(tp, i-1);
			if(next >= 0)
			{	//If there is another note in this track
				if(tp->note[i-1]->pos == tp->note[next]->pos)
				{	//If this note and the next are at the same position, merge them
					unsigned char lower = 0;	//Is set to nonzero if the next note has a lower fret value

					//Determine which of the two notes uses the lowest fret value, as this determines which note's end of pitched/unpitched slide positions are kept
					if(eof_pro_guitar_note_lowest_fret(tp, next) < eof_pro_guitar_note_lowest_fret(tp, i - 1))
					{	//If the next note uses a lower fret value
						lower = 1;	//Note that next note's end of pitched/unpitched slide takes priority
					}

					tp->note[i-1]->note |= tp->note[next]->note;	//Merge the two notes' bitmasks

					//Perform additional merging logic for pro guitar tracks, because Rocksmith custom files define single notes and chords at the same position in order to define chord techniques
					if(tp->note[next]->flags & (EOF_PRO_GUITAR_NOTE_FLAG_SLIDE_UP | EOF_PRO_GUITAR_NOTE_FLAG_SLIDE_DOWN))
					{	//If the next note is a slide
						if(lower)
						{	//If the next note's slide end position is to be used
							tp->note[i-1]->slideend = tp->note[next]->slideend;	//Copy the slide end position
						}
					}
					if(tp->note[next]->flags & EOF_PRO_GUITAR_NOTE_FLAG_BEND)
					{	//If the next note is a bend
						tp->note[i-1]->bendstrength = tp->note[next]->bendstrength;	//Copy the bend strength
					}
					if(tp->note[next]->flags & EOF_PRO_GUITAR_NOTE_FLAG_UNPITCH_SLIDE)
					{	//If the next note is an unpitched slide
						if(lower)
						{	//If the next note's unpitched slide end position is to be used
							tp->note[i-1]->unpitchend = tp->note[next]->unpitchend;	//Copy the unpitched slide end position
						}
					}
					flags = eof_prepare_note_flag_merge(tp->note[i-1]->flags, EOF_PRO_GUITAR_TRACK_BEHAVIOR, tp->note[next]->note);
					//Get the flags of the overlapped note as they would be if all applicable lane-specific flags are cleared to inherit the flags of the note to merge
					flags |= tp->note[next]->flags;	//Merge the next note's flags
					if((tp->note[i-1]->flags & EOF_PRO_GUITAR_NOTE_FLAG_SPLIT) != (tp->note[next]->flags & EOF_PRO_GUITAR_NOTE_FLAG_SPLIT))
					{	//If the two merged notes had dislike split status
						flags &= ~EOF_PRO_GUITAR_NOTE_FLAG_SPLIT;	//Remove split status from the merged flags to allow it to be nicely imported from RS files
					}
					tp->note[i-1]->flags = flags;	//Update the flags for the merged note
					if(tp->note[next]->length > tp->note[i-1]->length)
					{	//If the next note is longer
						tp->note[i-1]->length = tp->note[next]->length;	//Update the length for the merged note
					}

					for(ctr = 0, bitmask = 1; ctr < 6; ctr++, bitmask <<= 1)
					{	//For each of the next note's 6 usable strings
						if(tp->note[next]->note & bitmask)
						{	//If this string is used
							tp->note[i-1]->frets[ctr] = tp->note[next]->frets[ctr];	//Overwrite this note's fret value on this string with that of the next note
							tp->note[i-1]->finger[ctr] = tp->note[next]->finger[ctr];	//Overwrite this note's fingering on this string with that of the next note
						}
					}
					eof_pro_guitar_track_delete_note(tp, next);
				}//If this note and the next are at the same position, merge them
				else
				{	//Otherwise ensure one doesn't overlap the other improperly
					if(!eof_menu_track_get_tech_view_state(sp, track))
					{	//Only perform length validation when tech view isn't in effect, since tech notes cannot retain a specific length
						has_link_next = 0;	//Reset this condition
						for(ctr = 0, bitmask = 1; ctr < 6; ctr++, bitmask <<= 1)
						{	//For each of the 6 supported strings
							if(tp->note[i-1]->note & bitmask)
							{	//If this string is used
								(void) eof_get_rs_techniques(sp, track, i - 1, ctr, &ptr, 2, 1);	//Get the statuses in effect for this string, checking all applicable tech notes
								if(ptr.linknext)
								{	//If this string used linkNext status
									has_link_next = 1;
									break;	//The other strings don't need to be checked
								}
							}
						}
						if(has_link_next)
						{	//If the note has linkNext status, force the maximum length (up until the next note on that string occurs)
							if((tp->note[next]->note & tp->note[i-1]->note) == 0)
							{	//If this note and the next note have no strings in common
								tp->note[i-1]->flags |= EOF_NOTE_FLAG_CRAZY;	//This note will overlap at least one note, apply crazy status
							}
							maxlength = eof_get_note_max_length(sp, track, i - 1, 0);	//Determine the maximum length for this note, taking its crazy status into account and disregarding the minimum distance between notes
							eof_set_note_length(sp, track, i - 1, maxlength);	//Extend it to the next note that has a gem on a matching string
						}
						else
						{	//Otherwise enforce the minimum distance between the two notes
							maxlength = eof_get_note_max_length(sp, track, i - 1, 1);	//Determine the maximum length for this note, taking its crazy status into account
							if(maxlength && (eof_get_note_length(sp, track, i - 1) > maxlength))
							{	//If the note is longer than its maximum length
								eof_set_note_length(sp, track, i - 1, maxlength);	//Truncate it to its valid maximum length
							}
						}
					}
				}
			}//If there is another note in this track

			/* make sure that there aren't any invalid fret/finger values, and inspect the mute flag status */
			for(ctr = 0, allmuted = 1, bitmask = 1; ctr < 6; ctr++, bitmask<<=1)
			{	//For each of the 6 usable strings
				fretvalue = tp->note[i-1]->frets[ctr];
				if(fretvalue != 0xFF)
				{	//Track whether the all used strings in this note/chord are muted
					if(tp->note[i-1]->note & bitmask)
					{	//If this string is used in the note
						if(!(fretvalue & 0x80))
						{	//If the note isn't marked as muted
							allmuted = 0;
						}
					}
					else
					{	//If this string is not used in the note
						if(tp->note[i-1]->finger[ctr])
						{	//But the fingering is defined
							memset(tp->note[i-1]->finger, 0, 8);	//Initialize all fingers to undefined
						}
					}
					if((fretvalue & 0x7F) > tp->numfrets)
					{	//If this fret value is invalid
						tp->note[i-1]->frets[ctr] = 0;	//Revert to default fret value of 0
					}
				}
			}

			/* correct the mute flag status if necessary */
			if(!allmuted)
			{	//If any used strings in this note/chord weren't string muted
				tp->note[i-1]->flags &= (~EOF_PRO_GUITAR_NOTE_FLAG_STRING_MUTE);	//Clear the string mute flag
			}
			else if(!(tp->note[i-1]->flags & EOF_PRO_GUITAR_NOTE_FLAG_PALM_MUTE))
			{	//If all strings are muted and the user didn't specify a palm mute
				tp->note[i-1]->flags |= EOF_PRO_GUITAR_NOTE_FLAG_STRING_MUTE;		//Set the string mute flag
			}

			/* ensure that a note isn't both ghosted AND string muted (use conflicting channels during RB3 export) */
			if((tp->note[i-1]->flags & EOF_PRO_GUITAR_NOTE_FLAG_STRING_MUTE) && tp->note[i-1]->ghost)
			{	//If this note is string muted and any strings are ghosted
				tp->note[i-1]->ghost = 0;	//Remove ghost status from all strings
			}

			/* cleanup Rocksmith related slide/bend statuses */
			if((tp->note[i-1]->slideend > tp->numfrets) || !(tp->note[i-1]->flags & EOF_PRO_GUITAR_NOTE_FLAG_RS_NOTATION))
			{	//If the slide end position is invalid or not indicated to be defined
				tp->note[i-1]->slideend = 0;	//Clear it
			}
			if(tp->note[i-1]->bendstrength && !(tp->note[i-1]->flags & EOF_PRO_GUITAR_NOTE_FLAG_RS_NOTATION))
			{	//If the bend strength is invalid or not indicated to be defined
				tp->note[i-1]->bendstrength = 0;	//Clear it
			}
			if((tp->note[i-1]->unpitchend > tp->numfrets) || !tp->note[i-1]->unpitchend)
			{	//If the unpitched slide end position is invalid or not indicated to be defined
				tp->note[i-1]->unpitchend = 0;	//Clear it
				tp->note[i-1]->flags &= ~EOF_PRO_GUITAR_NOTE_FLAG_UNPITCH_SLIDE;	//Clear the related flag
			}
			if(!tp->note[i-1]->slideend && !tp->note[i-1]->bendstrength)
			{	//If this note has no slide end fret number or bend strength defined, or neither are indicated to be defined
				if(!((tp->note == tp->technote) && (tp->note[i-1]->flags & EOF_PRO_GUITAR_NOTE_FLAG_BEND)))
				{	//If tech view is in effect, a bend note is allowed to have a bend strength of 0
					tp->note[i-1]->flags &= ~EOF_PRO_GUITAR_NOTE_FLAG_RS_NOTATION;	//Otherwise both the slide end position and the bend strength are undefined, clear this flag
				}
			}
			if(!(tp->note[i-1]->flags & EOF_PRO_GUITAR_NOTE_FLAG_SLIDE_UP) && !(tp->note[i-1]->flags & EOF_PRO_GUITAR_NOTE_FLAG_SLIDE_DOWN) && !(tp->note[i-1]->flags & EOF_PRO_GUITAR_NOTE_FLAG_BEND))
			{	//If the note is not a slide or bend
				tp->note[i-1]->flags &= ~EOF_PRO_GUITAR_NOTE_FLAG_RS_NOTATION;	//Clear this flag
			}

			/* ensure that a note doesn't specify that an unused string is ghosted */
			tp->note[i-1]->ghost &= tp->note[i-1]->note;	//Clear all lanes that are specified by the note bitmask as being used
		}//If the note is valid, perform other cleanup
	}//For each note in the track

	//Run another pass to check crazy notes overlapping with gems on their same lanes more than 1 note ahead
	for(i = 0; i < tp->notes; i++)
	{	//For each note
		if(tp->note[i]->flags & EOF_NOTE_FLAG_CRAZY)
		{	//If this note is crazy, find the next gem that occupies any of the same lanes
			has_link_next = 0;	//Reset this status
			for(ctr = 0, bitmask = 1; ctr < 6; ctr++, bitmask <<= 1)
			{	//For each of the 6 supported strings
				if(tp->note[i]->note & bitmask)
				{	//If this string is used
					(void) eof_get_rs_techniques(sp, track, i, ctr, &ptr, 2, 1);	//Get the statuses in effect for this string, checking all applicable tech notes
					if(ptr.linknext)
					{	//If this string used linkNext status
						has_link_next = 1;
						break;	//The other strings don't need to be checked
					}
				}
			}
			if(!has_link_next)
			{	//Only enforce a 1ms minimum gap between this note and the next if this note doesn't have linkNext status
				next = i;
				while(1)
				{
					next = eof_fixup_next_pro_guitar_note(tp, next);
					if(next >= 0)
					{	//If there's a note in this difficulty after this note
						if(tp->note[i]->note & tp->note[next]->note)
						{	//And it uses at least one of the same lanes as the crazy note being checked
							if(tp->note[i]->pos + tp->note[i]->length >= tp->note[next]->pos - 1)
							{	//If it does not end at least 1ms before the next note starts
								tp->note[i]->length = tp->note[next]->pos - tp->note[i]->pos - 1;	//Truncate the crazy note so it does not overlap the next gem on its lane(s)
							}
							break;
						}
					}
					else
						break;
				}
			}
		}
	}

	for(i = 0; i < tp->tremolos; i++)
	{	//For each tremolo section in the track
		for(ctr = 0; ctr < tp->notes; ctr++)
		{	//For each note in the track
			if((tp->note[ctr]->pos >= tp->tremolo[i].start_pos) && (tp->note[ctr]->pos <= tp->tremolo[i].end_pos))
			{	//If the note is within the tremolo phrase
				if((tp->tremolo[i].difficulty == 0xFF) || (tp->tremolo[i].difficulty == tp->note[ctr]->type))
				{	//If the tremolo phrase applies to all difficulties or if it applies to the note's difficulty
					tp->note[ctr]->flags &= ~(EOF_PRO_GUITAR_NOTE_FLAG_HO);	//Clear the hammer on flag, which RB3 charts set needlessly
				}
			}
		}
	}

	eof_pro_guitar_track_fixup_hand_positions(sp, track);	//Cleanup fret hand positions

	//Ensure that the note at the beginning of each arpeggio/handshape phrase is authored correctly
	if(eof_write_rs_files || eof_write_rs2_files || eof_write_bf_files)
	{	//If the user wants to save Rocksmith or Bandfuse capable files
		char restore_tech_view = 0;			//If tech view is in effect, it is temporarily disabled so that tech notes don't get added instead of regular notes

		restore_tech_view = eof_menu_track_get_tech_view_state(sp, track);
		eof_menu_track_set_tech_view_state(sp, track, 0);	//Disable tech view if applicable
		for(ctr = 0; ctr < tp->arpeggios; ctr++)
		{	//For each arpeggio/handshape phrase in the track (outer for loop)
			for(ctr2 = 0; ctr2 < tp->notes; ctr2++)
			{	//For each note in the track (inner for loop)
				if(((tp->note[ctr2]->pos >= tp->arpeggio[ctr].start_pos) && (tp->note[ctr2]->pos <= tp->arpeggio[ctr].end_pos)) && (tp->note[ctr2]->type == tp->arpeggio[ctr].difficulty))
				{	//If this note is within the phrase
					if(tp->note[ctr2]->pos > tp->arpeggio[ctr].start_pos + 10)
					{	//If the note is later than 10ms after the start of the phrase
						np = eof_track_add_create_note(sp, track, tp->note[ctr2]->note, tp->arpeggio[ctr].start_pos, tp->note[ctr2]->length, tp->note[ctr2]->type, NULL);	//Initialize a new ghost note at the phrase's position
						if(np)
						{	//If the new note was created
							np->ghost = np->note;	//Make all used gems ghosted
							memcpy(np->frets, tp->note[ctr2]->frets, sizeof(np->frets));	//Clone the fret values
						}
					}
					break;	//Break inner for loop after processing the first note in the phrase
				}
			}
		}
		eof_menu_track_set_tech_view_state(sp, track, restore_tech_view);	//Re-enable tech view if applicable
		eof_track_sort_notes(sp, track);

		for(ctr = 0; ctr < tp->arpeggios; ctr++)
		{	//For each arpeggio phrase in the track (outer for loop)
			for(ctr2 = 0; ctr2 < tp->notes; ctr2++)
			{	//For each note in the track (inner for loop)
				if((tp->note[ctr2]->pos >= tp->arpeggio[ctr].start_pos) && (tp->note[ctr2]->pos <= tp->arpeggio[ctr].start_pos + 10) && (tp->note[ctr2]->type == tp->arpeggio[ctr].difficulty))
				{	//If this note's start position is at or up to 10ms after the start of an arpeggio phrase in this track difficulty
					unsigned char frets[6];	//Will be used to build a new fret array
					unsigned char note;		//Will be used to build a new note bitmask
					unsigned char ghost;	//Will be used to build a new ghost bitmask

					memcpy(frets, tp->note[ctr2]->frets, 6);	//Duplicate the original note's fret array
					note = tp->note[ctr2]->note;
					ghost = tp->note[ctr2]->ghost;
					nextnote = ctr2;
					while(1)
					{
						nextnote = eof_track_fixup_next_note(sp, track, nextnote);	//Iterate to the next note in this track difficulty
						if((nextnote >= 0) && (tp->note[nextnote]->pos <= tp->arpeggio[ctr].end_pos))
						{	//If there is another note and it is in the same arpeggio phrase
							for(ctr3 = 0, bitmask = 1; ctr3 < 6; ctr3++, bitmask <<= 1)
							{	//For each of the 6 supported strings
								if((tp->note[nextnote]->note & bitmask) && !(note & bitmask))
								{	//If the note uses a string that isn't yet defined as used in the arpeggio's base note
									note |= bitmask;	//Mark the string as used
									ghost |= bitmask;	//Mark it as ghosted
									frets[ctr3] = tp->note[nextnote]->frets[ctr3];	//Copy the gem's fret value
								}
							}
						}
						else
						{	//The next note (if any) is not in the arpeggio phrase
							break;	//Break from while loop
						}
					}
					if(eof_write_rs2_files && (eof_note_count_colors_bitmask(note) == 1))
					{	//If RS2 export is enabled, and the base note for the arpeggio only has one gem in it, add another gem to force it to become a chord (required for RS2 export)
						if(note == 1)
						{	//If that base note is on low E
							note |= 2;	//Add a gem on the next string up
							ghost |= 2;	//And ghost it
						}
						else
						{
							note |= 1;	//Otherwise add a gem on low E
							ghost |= 1;	//And ghost it
						}
					}
					memcpy(tp->note[ctr2]->frets, frets, 6);	//Apply changes (if any) to the note at the base of the arpeggio phrase
					tp->note[ctr2]->note = note;
					tp->note[ctr2]->ghost = ghost;
					break;	//Exit the inner for loop (no other notes in this track will be within the arpeggio phrase that was just parsed)
				}//If this note's start position is within 10ms of an arpeggio phrase in this track difficulty
			}//For each note in the track (inner for loop)
		}//For each arpeggio phrase in the track (outer for loop)
	}//If the user wants to save Rocksmith capable files

	if(tp->arrangement > 4)
	{	//If the track's arrangement type is invalid
		tp->arrangement = 0;	//Reset it to undefined
	}

	if(eof_enforce_chord_density)
	{	//If the user opted to automatically apply crazy status for repeated chords that are separated from their preceding chords by a rest
		eof_pro_guitar_track_enforce_chord_density(tp);
	}

	if(!sel)
	{
		if(eof_selection.current < tp->notes)
		{
			eof_selection.multi[eof_selection.current] = 1;
		}
	}

	eof_menu_pro_guitar_track_update_note_counter(tp);	//Update the pgnotes or technotes counter as appropriate
}

unsigned long eof_get_num_star_power_paths(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
		return sp->legacy_track[tracknum]->star_power_paths;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return sp->pro_guitar_track[tracknum]->star_power_paths;
	}

	return 0;	//Return error
}

EOF_PHRASE_SECTION *eof_get_star_power_path(EOF_SONG *sp, unsigned long track, unsigned long pathnum)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(pathnum < EOF_MAX_PHRASES)
			{
				if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
				{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
					tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
				}
				return &sp->legacy_track[tracknum]->star_power_path[pathnum];
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
		return NULL;	//Vocal star power is not implemented yet

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(pathnum < EOF_MAX_PHRASES)
			{
				return &sp->pro_guitar_track[tracknum]->star_power_path[pathnum];
			}
		break;
	}

	return NULL;	//Return error
}

void eof_set_num_solos(EOF_SONG *sp, unsigned long track, unsigned long number)
{
	unsigned long tracknum;

 	eof_log("eof_set_num_solos() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
			sp->legacy_track[tracknum]->solos = number;
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			sp->pro_guitar_track[tracknum]->solos = number;
		break;
	}
}

void eof_set_num_star_power_paths(EOF_SONG *sp, unsigned long track, unsigned long number)
{
// 	eof_log("eof_set_num_star_power_paths() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
			sp->legacy_track[tracknum]->star_power_paths = number;
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			sp->pro_guitar_track[tracknum]->star_power_paths = number;
		break;
	}
}

long eof_track_fixup_previous_note(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return -1;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
		return eof_fixup_previous_legacy_note(sp->legacy_track[tracknum], note);

		case EOF_VOCAL_TRACK_FORMAT:
		return eof_fixup_previous_lyric(sp->vocal_track[tracknum], note);

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return eof_fixup_previous_pro_guitar_note(sp->pro_guitar_track[tracknum], note);
	}

	return -1;
}

long eof_track_fixup_next_note(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return -1;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
		return eof_fixup_next_legacy_note(sp->legacy_track[tracknum], note);

		case EOF_VOCAL_TRACK_FORMAT:
		return eof_fixup_next_lyric(sp->vocal_track[tracknum], note);

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return eof_fixup_next_pro_guitar_note(sp->pro_guitar_track[tracknum], note);
	}

	return -1;
}

void eof_track_find_crazy_notes(EOF_SONG *sp, unsigned long track)
{
	unsigned long i;
	long next;

 	eof_log("eof_track_find_crazy_notes() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;

	if(sp->track[track]->track_format == EOF_VOCAL_TRACK_FORMAT)
		return;	//Vocal tracks don't have the capability to have "crazy" notes

	for(i = 0; i < eof_get_track_size(sp, track); i++)
	{	//For each note in the track
		next = eof_track_fixup_next_note(sp, track, i);
		if(next >= 0)
		{
			if(eof_get_note_pos(sp, track, i) + eof_get_note_length(sp, track, i) > eof_get_note_pos(sp, track, next) + 1)
			{	//If the note overlaps the next note by over one millisecond (to allow for rounding errors)
				if((eof_get_note_note(sp, track, i) & eof_get_note_note(sp, track, next)) == 0)
				{	//If the two notes have no lanes in common
					eof_set_note_flags(sp, track, i, eof_get_note_flags(sp, track, i) | EOF_NOTE_FLAG_CRAZY);
				}
			}
		}
	}
}

void eof_set_note_flags(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned long flags)
{
// 	eof_log("eof_set_note_flags() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				sp->legacy_track[tracknum]->note[note]->flags = flags;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				sp->vocal_track[tracknum]->lyric[note]->flags = flags;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				sp->pro_guitar_track[tracknum]->note[note]->flags = flags;
			}
		break;
	}
}

void eof_set_note_tflags(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned short tflags)
{
// 	eof_log("eof_set_note_flags() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				sp->legacy_track[tracknum]->note[note]->tflags = tflags;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				sp->vocal_track[tracknum]->lyric[note]->tflags = tflags;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				sp->pro_guitar_track[tracknum]->note[note]->tflags = tflags;
			}
		break;
	}
}

void eof_set_note_eflags(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned char eflags)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				sp->pro_guitar_track[tracknum]->note[note]->eflags = eflags;
			}
		break;
	}
}

void eof_set_note_type(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned char type)
{
// 	eof_log("eof_set_note_type() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				sp->legacy_track[tracknum]->note[note]->type = type;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				sp->vocal_track[tracknum]->lyric[note]->type = type;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				sp->pro_guitar_track[tracknum]->note[note]->type = type;
			}
		break;
	}
}

void eof_track_delete_star_power_path(EOF_SONG *sp, unsigned long track, unsigned long pathnum)
{
	unsigned long tracknum;

 	eof_log("eof_track_delete_star_power_path() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
			eof_legacy_track_delete_star_power(sp->legacy_track[tracknum], pathnum);
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			eof_pro_guitar_track_delete_star_power(sp->pro_guitar_track[tracknum], pathnum);
		break;
	}
}

void eof_pro_guitar_track_delete_star_power(EOF_PRO_GUITAR_TRACK * tp, unsigned long index)
{
	unsigned long i;

	if(!tp || (index >= tp->star_power_paths))
		return;

	tp->star_power_path[index].name[0] = '\0';	//Empty the name string
	for(i = index; i < tp->star_power_paths - 1; i++)
	{
		memcpy(&tp->star_power_path[i], &tp->star_power_path[i + 1], sizeof(EOF_PHRASE_SECTION));
	}
	tp->star_power_paths--;
}

int eof_track_add_star_power_path(EOF_SONG *sp, unsigned long track, unsigned long start_pos, unsigned long end_pos)
{
	unsigned long tracknum;

 	eof_log("eof_track_add_star_power_path() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
		return eof_legacy_track_add_star_power(sp->legacy_track[tracknum], start_pos, end_pos);

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return eof_pro_guitar_track_add_star_power(sp->pro_guitar_track[tracknum], start_pos, end_pos);

		case EOF_VOCAL_TRACK_FORMAT:
		return eof_vocal_track_add_star_power(sp->vocal_track[tracknum], start_pos, end_pos);
	}
	return 0;	//Return error
}

int eof_pro_guitar_track_add_star_power(EOF_PRO_GUITAR_TRACK * tp, unsigned long start_pos, unsigned long end_pos)
{
	if(tp && (tp->star_power_paths < EOF_MAX_PHRASES))
	{	//If the maximum number of star power phrases for this track hasn't already been defined
		tp->star_power_path[tp->star_power_paths].start_pos = start_pos;
		tp->star_power_path[tp->star_power_paths].end_pos = end_pos;
		tp->star_power_path[tp->star_power_paths].flags = 0;
		tp->star_power_path[tp->star_power_paths].name[0] = '\0';
		tp->star_power_paths++;
		return 1;	//Return success
	}
	return 0;	//Return error
}

void eof_track_delete_solo(EOF_SONG *sp, unsigned long track, unsigned long pathnum)
{
	unsigned long tracknum;

 	eof_log("eof_track_delete_solo() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
			eof_legacy_track_delete_solo(sp->legacy_track[tracknum], pathnum);
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			eof_pro_guitar_track_delete_solo(sp->pro_guitar_track[tracknum], pathnum);
		break;
	}
}

void eof_pro_guitar_track_delete_solo(EOF_PRO_GUITAR_TRACK * tp, unsigned long index)
{
	unsigned long i;

	if(!tp || (index >= tp->solos))
		return;

	tp->solo[index].name[0] = '\0';	//Empty the name string
	for(i = index; i < tp->solos - 1; i++)
	{
		memcpy(&tp->solo[i], &tp->solo[i + 1], sizeof(EOF_PHRASE_SECTION));
	}
	tp->solos--;
}

int eof_track_add_solo(EOF_SONG *sp, unsigned long track, unsigned long start_pos, unsigned long end_pos)
{
	unsigned long tracknum;

 	eof_log("eof_track_add_solo() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
		return eof_legacy_track_add_solo(sp->legacy_track[tracknum], start_pos, end_pos);

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return eof_pro_guitar_track_add_solo(sp->pro_guitar_track[tracknum], start_pos, end_pos);
	}
	return 0;	//Return error
}

int eof_pro_guitar_track_add_solo(EOF_PRO_GUITAR_TRACK * tp, unsigned long start_pos, unsigned long end_pos)
{
	if(tp && (tp->solos < EOF_MAX_PHRASES))
	{	//If the maximum number of solo phrases for this track hasn't already been defined
		tp->solo[tp->solos].start_pos = start_pos;
		tp->solo[tp->solos].end_pos = end_pos;
		tp->solo[tp->solos].flags = 0;
		tp->solo[tp->solos].name[0] = '\0';
		tp->solos++;
		return 1;	//Return success
	}
	return 0;	//Return error
}

void eof_note_set_tail_pos(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned long pos)
{
 	eof_log("eof_note_set_tail_pos() entered", 2);

	if((sp == NULL) || (track >= sp->tracks) || (note >= eof_get_track_size(sp, track)))
		return;

	eof_set_note_length(sp, track, note, pos - eof_get_note_pos(sp, track, note));
}

unsigned long eof_get_used_lanes(unsigned long track, unsigned long startpos, unsigned long endpos, char type)
{
	unsigned long ctr, bitmask = 0;

	for(ctr = 0; ctr < eof_get_track_size(eof_song, track); ctr++)
	{	//For each note in the specified track
		if((eof_get_note_type(eof_song, track, ctr) == type) && (eof_get_note_pos(eof_song, track, ctr) >= startpos) && (eof_get_note_pos(eof_song, track, ctr) <= endpos))
		{	//If the note is in the specified difficulty and is within the specified time range
			bitmask |= eof_get_note_note(eof_song, track, ctr);	//Add its bitmask to the result
		}
	}

	return bitmask;
}

unsigned long eof_get_num_trills(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
		return sp->legacy_track[tracknum]->trills;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return sp->pro_guitar_track[tracknum]->trills;
	}

	return 0;	//Return error
}

unsigned long eof_get_num_tremolos(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
		return sp->legacy_track[tracknum]->tremolos;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return sp->pro_guitar_track[tracknum]->tremolos;
	}

	return 0;	//Return error
}

unsigned long eof_get_num_sliders(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
		return sp->legacy_track[tracknum]->sliders;
	}

	return 0;	//Return error
}

EOF_PHRASE_SECTION *eof_get_trill(EOF_SONG *sp, unsigned long track, unsigned long index)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(index < EOF_MAX_PHRASES)
			{
				if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
				{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
					tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
				}
				return &sp->legacy_track[tracknum]->trill[index];
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(index < EOF_MAX_PHRASES)
			{
				return &sp->pro_guitar_track[tracknum]->trill[index];
			}
		break;
	}

	return NULL;	//Return error
}

EOF_PHRASE_SECTION *eof_get_tremolo(EOF_SONG *sp, unsigned long track, unsigned long index)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(index < EOF_MAX_PHRASES)
			{
				if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
				{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
					tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
				}
				return &sp->legacy_track[tracknum]->tremolo[index];
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(index < EOF_MAX_PHRASES)
			{
				return &sp->pro_guitar_track[tracknum]->tremolo[index];
			}
		break;
	}

	return NULL;	//Return error
}

EOF_PHRASE_SECTION *eof_get_slider(EOF_SONG *sp, unsigned long track, unsigned long index)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(index < EOF_MAX_PHRASES)
			{
				return &sp->legacy_track[tracknum]->slider[index];
			}
		break;
	}
	return NULL;	//Return error
}

void eof_track_delete_trill(EOF_SONG *sp, unsigned long track, unsigned long pathnum)
{
	unsigned long tracknum;

 	eof_log("eof_track_delete_trill() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
			eof_legacy_track_delete_trill(sp->legacy_track[tracknum], pathnum);
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			eof_pro_guitar_track_delete_trill(sp->pro_guitar_track[tracknum], pathnum);
		break;
	}
}

void eof_legacy_track_delete_trill(EOF_LEGACY_TRACK * tp, unsigned long index)
{
	unsigned long i;

	if(!tp || (index >= tp->trills))
		return;

	tp->trill[index].name[0] = '\0';	//Empty the name string
	for(i = index; i < tp->trills - 1; i++)
	{
		memcpy(&tp->trill[i], &tp->trill[i + 1], sizeof(EOF_PHRASE_SECTION));
	}
	tp->trills--;
}

void eof_pro_guitar_track_delete_trill(EOF_PRO_GUITAR_TRACK * tp, unsigned long index)
{
	unsigned long i;

	if(!tp || (index >= tp->trills))
		return;

	tp->trill[index].name[0] = '\0';	//Empty the name string
	for(i = index; i < tp->trills - 1; i++)
	{
		memcpy(&tp->trill[i], &tp->trill[i + 1], sizeof(EOF_PHRASE_SECTION));
	}
	tp->trills--;
}

int eof_track_add_trill(EOF_SONG *sp, unsigned long track, unsigned long start_pos, unsigned long end_pos)
{
	unsigned long tracknum;

 	eof_log("eof_track_add_trill() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
		return eof_legacy_track_add_trill(sp->legacy_track[tracknum], start_pos, end_pos);

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return eof_pro_guitar_track_add_trill(sp->pro_guitar_track[tracknum], start_pos, end_pos);
	}
	return 0;	//Return error
}

int eof_legacy_track_add_trill(EOF_LEGACY_TRACK * tp, unsigned long start_pos, unsigned long end_pos)
{
	if(tp && (tp->trills < EOF_MAX_PHRASES))
	{	//If the maximum number of solo phrases for this track hasn't already been defined
		tp->trill[tp->trills].start_pos = start_pos;
		tp->trill[tp->trills].end_pos = end_pos;
		tp->trill[tp->trills].flags = 0;
		tp->trill[tp->trills].name[0] = '\0';
		tp->trills++;
		return 1;	//Return success
	}
	return 0;	//Return error
}

int eof_pro_guitar_track_add_trill(EOF_PRO_GUITAR_TRACK * tp, unsigned long start_pos, unsigned long end_pos)
{
	if(tp && (tp->trills < EOF_MAX_PHRASES))
	{	//If the maximum number of solo phrases for this track hasn't already been defined
		tp->trill[tp->trills].start_pos = start_pos;
		tp->trill[tp->trills].end_pos = end_pos;
		tp->trill[tp->trills].flags = 0;
		tp->trill[tp->trills].name[0] = '\0';
		tp->trills++;
		return 1;	//Return success
	}
	return 0;	//Return error
}

void eof_track_delete_tremolo(EOF_SONG *sp, unsigned long track, unsigned long index)
{
	unsigned long tracknum;

 	eof_log("eof_track_delete_tremolo() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
			eof_legacy_track_delete_tremolo(sp->legacy_track[tracknum], index);
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			eof_pro_guitar_track_delete_tremolo(sp->pro_guitar_track[tracknum], index);
		break;
	}
}

void eof_legacy_track_delete_tremolo(EOF_LEGACY_TRACK * tp, unsigned long index)
{
	unsigned long i;

	if(!tp || (index >= tp->tremolos))
		return;

	tp->tremolo[index].name[0] = '\0';	//Empty the name string
	for(i = index; i < tp->tremolos - 1; i++)
	{
		memcpy(&tp->tremolo[i], &tp->tremolo[i + 1], sizeof(EOF_PHRASE_SECTION));
	}
	tp->tremolos--;
}

void eof_pro_guitar_track_delete_tremolo(EOF_PRO_GUITAR_TRACK * tp, unsigned long index)
{
	unsigned long i;

	if(!tp || (index >= tp->tremolos))
		return;

	tp->tremolo[index].name[0] = '\0';	//Empty the name string
	for(i = index; i < tp->tremolos - 1; i++)
	{
		memcpy(&tp->tremolo[i], &tp->tremolo[i + 1], sizeof(EOF_PHRASE_SECTION));
	}
	tp->tremolos--;
}

int eof_track_add_tremolo(EOF_SONG *sp, unsigned long track, unsigned long start_pos, unsigned long end_pos, unsigned char diff)
{
	unsigned long tracknum;

 	eof_log("eof_track_add_tremolo() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
		return eof_legacy_track_add_tremolo(sp->legacy_track[tracknum], start_pos, end_pos);

		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return eof_pro_guitar_track_add_tremolo(sp->pro_guitar_track[tracknum], start_pos, end_pos, diff);
	}
	return 0;	//Return error
}

int eof_legacy_track_add_tremolo(EOF_LEGACY_TRACK * tp, unsigned long start_pos, unsigned long end_pos)
{
	if(tp && (tp->tremolos < EOF_MAX_PHRASES))
	{	//If the maximum number of solo phrases for this track hasn't already been defined
		tp->tremolo[tp->tremolos].start_pos = start_pos;
		tp->tremolo[tp->tremolos].end_pos = end_pos;
		tp->tremolo[tp->tremolos].flags = 0;
		tp->tremolo[tp->tremolos].name[0] = '\0';
		tp->tremolo[tp->tremolos].difficulty = 0xFF;	//In legacy tracks, tremolo sections always apply to all difficulties
		tp->tremolos++;
		return 1;	//Return success
	}
	return 0;	//Return error
}

int eof_pro_guitar_track_add_tremolo(EOF_PRO_GUITAR_TRACK * tp, unsigned long start_pos, unsigned long end_pos, unsigned char diff)
{
	if(tp && (tp->tremolos < EOF_MAX_PHRASES))
	{	//If the maximum number of solo phrases for this track hasn't already been defined
		tp->tremolo[tp->tremolos].start_pos = start_pos;
		tp->tremolo[tp->tremolos].end_pos = end_pos;
		tp->tremolo[tp->tremolos].flags = 0;
		tp->tremolo[tp->tremolos].name[0] = '\0';
		tp->tremolo[tp->tremolos].difficulty = diff;
		tp->tremolos++;
		return 1;	//Return success
	}
	return 0;	//Return error
}

void eof_track_delete_slider(EOF_SONG *sp, unsigned long track, unsigned long index)
{
	unsigned long ctr;
	unsigned long tracknum;

 	eof_log("eof_track_delete_slider() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(index < sp->legacy_track[tracknum]->sliders)
			{
				sp->legacy_track[tracknum]->slider[index].name[0] = '\0';	//Empty the name string
				for(ctr = index; ctr < sp->legacy_track[tracknum]->sliders; ctr++)
				{
					memcpy(&sp->legacy_track[tracknum]->slider[ctr], &sp->legacy_track[tracknum]->slider[ctr + 1], sizeof(EOF_PHRASE_SECTION));
				}
				sp->legacy_track[tracknum]->sliders--;
			}
		break;
	}
}

void eof_set_num_trills(EOF_SONG *sp, unsigned long track, unsigned long number)
{
	unsigned long tracknum;

 	eof_log("eof_set_num_trills() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
			sp->legacy_track[tracknum]->trills = number;
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			sp->pro_guitar_track[tracknum]->trills = number;
		break;
	}
}

void eof_set_num_tremolos(EOF_SONG *sp, unsigned long track, unsigned long number)
{
	unsigned long tracknum;

 	eof_log("eof_set_num_tremolos() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if((sp->track[track]->track_type == EOF_TRACK_DRUM_PS) && !sp->tags->unshare_drum_phrasing)
			{	//If the specified track is the Phase Shift drum track, it refers to the normal drum track phrasing by default
				tracknum = sp->track[EOF_TRACK_DRUM]->tracknum;
			}
			sp->legacy_track[tracknum]->tremolos = number;
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			sp->pro_guitar_track[tracknum]->tremolos = number;
		break;
	}
}

void eof_set_num_sliders(EOF_SONG *sp, unsigned long track, unsigned long number)
{
	unsigned long tracknum;

 	eof_log("eof_set_num_sliders() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			sp->legacy_track[tracknum]->sliders = number;
		break;
	}
}

void eof_set_pro_guitar_fret_number(char function, unsigned long fretvalue)
{
	unsigned long ctr, ctr2, bitmask, tracknum;
	char undo_made = 0;
	unsigned char oldfretvalue = 0, newfretvalue = 0;
	int note_selection_updated;

 	eof_log("eof_set_pro_guitar_fret_number() entered", 1);

	if(eof_song->track[eof_selected_track]->track_format != EOF_PRO_GUITAR_TRACK_FORMAT)
		return;	//Do not allow this function to run unless a pro guitar/bass track is active

	if(eof_menu_track_get_tech_view_state(eof_song, eof_selected_track))
		return;	//Don't allow this function to run if tech view is in effect, since tech notes completely disregard their fret values

	note_selection_updated = eof_feedback_mode_update_note_selection();	//If no notes are selected, select the seek hover note if Feedback input mode is in effect
	tracknum = eof_song->track[eof_selected_track]->tracknum;
	for(ctr = 0; ctr < eof_song->pro_guitar_track[tracknum]->notes; ctr++)
	{	//For each note in the active pro guitar track
		if((eof_selection.track == eof_selected_track) && eof_selection.multi[ctr] && (eof_song->pro_guitar_track[tracknum]->note[ctr]->type == eof_note_type))
		{	//If the note is selected and is in the active difficulty
			for(ctr2 = 0, bitmask = 1; ctr2 < 6; ctr2++, bitmask<<=1)
			{	//For each of the 6 usable strings
				if((eof_song->pro_guitar_track[tracknum]->note[ctr]->note & bitmask) && (eof_pro_guitar_fret_bitmask & bitmask))
				{	//If this string is in use, and this string is enabled for fret shortcut manipulation
					oldfretvalue = eof_song->pro_guitar_track[tracknum]->note[ctr]->frets[ctr2];
					newfretvalue = oldfretvalue;
					switch(function)
					{
						case 0:	//Set fret value
							newfretvalue = fretvalue;
						break;

						case 1:	//Increment fret value
							if(oldfretvalue != 0xFF)	//Don't increment a muted note
								newfretvalue++;
						break;

						case 2:	//Decrement fret value
							if(oldfretvalue > 0)	//Don't decrement an open note
								newfretvalue--;
						break;
					}
					if((newfretvalue <= eof_song->pro_guitar_track[tracknum]->numfrets) || (newfretvalue == 0xFF))
					{	//Only set the fret value if it is valid
						if(!undo_made && (newfretvalue != oldfretvalue))
						{	//Make an undo state before making the first change
							eof_prepare_undo(EOF_UNDO_TYPE_NONE);
							undo_made = 1;
						}
						eof_song->pro_guitar_track[tracknum]->note[ctr]->frets[ctr2] = newfretvalue;	//Update the string's fret value
						memset(eof_song->pro_guitar_track[tracknum]->note[ctr]->finger, 0, 8);			//Initialize all fingers to undefined
					}
				}
			}
		}
	}
	if(undo_made)
	{	//If there were changes made, run the note cleanup to correct the string mute statuses, etc.
		eof_track_fixup_notes(eof_song, eof_selected_track, 1);
	}
	if(note_selection_updated)
	{	//If the only note modified was the seek hover note
		eof_selection.multi[eof_seek_hover_note] = 0;	//Deselect it to restore the note selection's original condition
		eof_selection.current = EOF_MAX_NOTES - 1;
	}
}

char *eof_get_note_name(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				return sp->legacy_track[tracknum]->note[note]->name;
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				return sp->vocal_track[tracknum]->lyric[note]->text;
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				return sp->pro_guitar_track[tracknum]->note[note]->name;
			}
		break;
	}

	return NULL;	//Return error
}

void *eof_copy_note(EOF_SONG *sp, unsigned long sourcetrack, unsigned long sourcenote, unsigned long desttrack, unsigned long pos, long length, char type)
{
	unsigned long sourcetracknum, desttracknum, newnotenum;
	unsigned long note, flags;
	char *text;
	void *result = NULL;

 	eof_log("eof_copy_note() entered", 2);

	//Validate parameters
	if((sp == NULL) || (sourcetrack >= sp->tracks) || (desttrack >= sp->tracks) || (sourcenote >= eof_get_track_size(sp, sourcetrack)))
		return NULL;

	//Don't allow copying instrument track notes to PART VOCALS and vice versa
	if(((sourcetrack == EOF_TRACK_VOCALS) && (desttrack != EOF_TRACK_VOCALS)) || ((sourcetrack != EOF_TRACK_VOCALS) && (desttrack == EOF_TRACK_VOCALS)))
		return NULL;

	sourcetracknum = sp->track[sourcetrack]->tracknum;
	desttracknum = sp->track[desttrack]->tracknum;

	note = eof_get_note_note(sp, sourcetrack, sourcenote);
	text = eof_get_note_name(sp, sourcetrack, sourcenote);
	flags = eof_get_note_flags(sp, sourcetrack, sourcenote);

	if(desttrack == EOF_TRACK_VOCALS)
	{	//If copying from PART VOCALS
		return eof_track_add_create_note(sp, desttrack, note, pos, length, type, text);
	}
	else
	{	//If copying from a non vocal track
		if((sp->track[sourcetrack]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT) && (sp->track[desttrack]->track_format != EOF_PRO_GUITAR_TRACK_FORMAT))
		{	//If copying from a pro guitar track to a non pro guitar track
			if(sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->legacymask != 0)
			{	//If the user defined how this pro guitar note would transcribe to a legacy track
				note = sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->legacymask;	//Use that bitmask
			}
		}

		result = eof_track_add_create_note(sp, desttrack, note, pos, length, type, text);
		if(result)
		{	//If the note was successfully created
			newnotenum = eof_get_track_size(sp, desttrack) - 1;		//The index of the new note
			eof_set_note_flags(sp, desttrack, newnotenum, flags);	//Copy the source note's flags to the newly created note
			if((sp->track[sourcetrack]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT) && (sp->track[desttrack]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT))
			{	//If the note was copied from a pro guitar track and pasted to a pro guitar track
				memcpy(sp->pro_guitar_track[desttracknum]->note[newnotenum]->frets, sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->frets, 6);		//Copy the six usable string fret values from the source note to the newly created note
				memcpy(sp->pro_guitar_track[desttracknum]->note[newnotenum]->finger, sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->finger, 6);	//Copy the six usable finger values from the source note to the newly created note
				sp->pro_guitar_track[desttracknum]->note[newnotenum]->ghost = sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->ghost;				//Copy the ghost bitmask
				sp->pro_guitar_track[desttracknum]->note[newnotenum]->legacymask = sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->legacymask;		//Copy the legacy bitmask
				sp->pro_guitar_track[desttracknum]->note[newnotenum]->bendstrength = sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->bendstrength;	//Copy the bend strength
				sp->pro_guitar_track[desttracknum]->note[newnotenum]->slideend = sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->slideend;			//Copy the slide end position
				sp->pro_guitar_track[desttracknum]->note[newnotenum]->unpitchend = sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->unpitchend;		//Copy the unpitched slide end position
				sp->pro_guitar_track[desttracknum]->note[newnotenum]->eflags = sp->pro_guitar_track[sourcetracknum]->note[sourcenote]->eflags;				//Copy the extended flags
			}
		}
	}//If copying from a non vocal track

	return result;
}

unsigned long eof_get_num_arpeggios(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return sp->pro_guitar_track[tracknum]->arpeggios;
	}

	return 0;	//Return error
}

EOF_PHRASE_SECTION *eof_get_arpeggio(EOF_SONG *sp, unsigned long track, unsigned long index)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(index < EOF_MAX_PHRASES)
			{
				return &sp->pro_guitar_track[tracknum]->arpeggio[index];
			}
		break;
	}

	return NULL;	//Return error
}

unsigned long eof_get_num_popup_messages(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return sp->pro_guitar_track[tracknum]->popupmessages;
	}

	return 0;	//Return error
}

unsigned long eof_get_num_fret_hand_positions(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
		return sp->pro_guitar_track[tracknum]->handpositions;
	}

	return 0;	//Return error
}

EOF_PHRASE_SECTION *eof_get_fret_hand_position(EOF_SONG *sp, unsigned long track, unsigned long index)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(index < EOF_MAX_PHRASES)
			{
				return &sp->pro_guitar_track[tracknum]->handposition[index];
			}
		break;
	}

	return NULL;	//Return error
}

void eof_set_num_fret_hand_positions(EOF_SONG *sp, unsigned long track, unsigned long number)
{
	unsigned long tracknum;

 	eof_log("eof_set_num_arpeggios() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
			sp->pro_guitar_track[tracknum]->handpositions = number;
		break;
	}
}

int eof_create_image_sequence(char benchmark_only)
{
	unsigned long framectr = 0, refreshctr = 0, lastpollctr = 0;
	unsigned long remainder = 0;
	char windowtitle[101] = {0};
	double fps = 0.0;
	clock_t curtime, lastpolltime = 0;
	int err;
	char filename[20] = {0};
	clock_t starttime = 0, endtime = 0;
	char *function = "Exporting";

	if(!eof_song_loaded || !eof_song)
		return 1;	//Do not allow this function to run if a chart is not loaded

	eof_log("eof_create_image_sequence() entered", 1);
	if(!benchmark_only)
	{	//If saving the image sequence to disk
		eof_log("\tCreating image sequence", 1);

		/* check to make sure \sequence folder exists */
		(void) ustrcpy(eof_temp_filename, eof_song_path);
		(void) replace_filename(eof_temp_filename, eof_temp_filename, "", (int)sizeof(eof_temp_filename));
		put_backslash(eof_temp_filename);
		(void) ustrcat(eof_temp_filename, "sequence");
		eof_clear_input();
		if(!file_exists(eof_temp_filename, FA_DIREC | FA_HIDDEN, NULL))
		{	//If this folder doesn't already exist
			err = eof_mkdir(eof_temp_filename);
			if(err)
			{	//If the folder could not be created
				allegro_message("Could not create folder!\n%s", eof_temp_filename);
				return 1;
			}
		}
		else if(alert(NULL, "Overwrite contents of existing \\sequence folder?", NULL, "&Yes", "&No", 'y', 'n') != 1)
		{	//If user declined to overwrite the contents of the folder
			return 1;
		}
		put_backslash(eof_temp_filename);	//eof_temp_filename is now the path of the \sequence folder
	}
	else
	{	//If performing a benchmark
		starttime = clock();	//Get the start time of the image sequence export
		function = "Benchmarking";
		eof_log("\tBenchmarking rendering performance", 1);
	}

	alogg_seek_abs_msecs_ogg(eof_music_track, 0);
	eof_music_actual_pos = alogg_get_pos_msecs_ogg(eof_music_track);
	eof_music_pos = eof_music_actual_pos + eof_av_delay;
	clear_to_color(eof_screen, eof_color_light_gray);
	blit(eof_image[EOF_IMAGE_MENU_FULL], eof_screen, 0, 0, 0, 0, eof_screen->w, eof_screen->h);
	while(eof_music_pos <  eof_music_length)
	{
		if(key[KEY_ESC])
			break;

		if(refreshctr >= 10)
		{
		//Update EOF's window title to provide a status
			curtime = clock();	//Get the current time
			fps = (double)(framectr - lastpollctr) / ((double)(curtime - lastpolltime) / (double)CLOCKS_PER_SEC);	//Find the number of FPS rendered since the last poll
			(void) snprintf(windowtitle, sizeof(windowtitle) - 1, "%s image sequence: %.2f%% (%.2fFPS) - Press Esc to cancel",function, (double)eof_music_pos/(double)eof_music_length * 100.0, fps);
			set_window_title(windowtitle);
			refreshctr -= 10;
			lastpolltime = curtime;
			lastpollctr = framectr;
		}

		eof_render();
		if(!benchmark_only)
		{	//If saving the image sequence to disk, export this frame to an image file
			(void) snprintf(filename, sizeof(filename) - 1, "%08lu.pcx",framectr);
			(void) replace_filename(eof_temp_filename, eof_temp_filename, filename, (int)sizeof(eof_temp_filename));
			(void) save_pcx(eof_temp_filename, eof_screen, NULL);	//Pass a NULL palette
		}

	//Seek one frame (1/30 second) further into the audio, tracking for rounding errors
		#define EOF_IMAGE_SEQUENCE_FPS 30
		framectr++;
		refreshctr++;
		eof_music_pos += 1000 / EOF_IMAGE_SEQUENCE_FPS;
		remainder += 1000 % EOF_IMAGE_SEQUENCE_FPS;	//Track the remainder
		while(remainder >= EOF_IMAGE_SEQUENCE_FPS)
		{
			eof_music_pos++;
			remainder -= EOF_IMAGE_SEQUENCE_FPS;
		}
	}

	if(benchmark_only)
	{	//If performing a benchmark, calculate and display the rendering performance
		endtime = clock();	//Get the start time of the image sequence export
		fps = (double)framectr / ((double)(endtime - starttime) / (double)CLOCKS_PER_SEC);	//Find the average FPS
		(void) snprintf(windowtitle, sizeof(windowtitle) - 1, "Average render rate was %.2fFPS",fps);
		allegro_message("%s", windowtitle);
		eof_log(windowtitle, 1);
	}

	eof_fix_window_title();

	if(!benchmark_only)
	{
		eof_log("\tImage sequence created", 1);
	}
	else
	{
		eof_log("\tBenchmark complete", 1);
	}

	return 1;
}

int eof_write_image_sequence(void)
{
	return eof_create_image_sequence(0);	//Generate the image sequence and write it to disk
}

int eof_benchmark_image_sequence(void)
{
	return eof_create_image_sequence(1);	//Generate the image sequence and only display it to screen
}

unsigned long eof_get_num_lyric_sections(EOF_SONG *sp, unsigned long track)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_VOCAL_TRACK_FORMAT:
		return sp->vocal_track[tracknum]->lines;
	}

	return 0;	//Return error
}

EOF_PHRASE_SECTION *eof_get_lyric_section(EOF_SONG *sp, unsigned long track, unsigned long sectionnum)
{
	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks))
		return NULL;	//Return error
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_VOCAL_TRACK_FORMAT:
			if(sectionnum < EOF_MAX_LYRIC_LINES)
			{
				return &sp->vocal_track[tracknum]->line[sectionnum];
			}
		break;
	}

	return NULL;	//Return error
}

void eof_adjust_note_length(EOF_SONG * sp, unsigned long track, unsigned long amount, int dir)
{
	unsigned long i, undo_made = 0, adjustment, notepos, notelength, newnotelength, newnotelength2;
	long next_note;
	int note_selection_updated;

 	eof_log("eof_adjust_note_length() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;

	if(sp->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
	{	//If a pro guitar track is being altered
		EOF_PRO_GUITAR_TRACK *tp = sp->pro_guitar_track[sp->track[track]->tracknum];
		if(tp->note == tp->technote)
		{	//If tech view is in effect for the track
			return;	//Do not allow this function to alter a tech note's length
		}
	}

	note_selection_updated = eof_feedback_mode_update_note_selection();	//If no notes are selected, select the seek hover note if Feedback input mode is in effect
	adjustment = amount;	//This would be the amount to adjust the note by
	for(i = 0; i < eof_get_track_size(sp, eof_selected_track); i++)
	{	//For each note in the track
		notepos = eof_get_note_pos(sp, track, i);
		notelength = eof_get_note_length(sp, track, i);

		if((eof_selection.track == eof_selected_track) && eof_selection.multi[i] && (eof_get_note_type(sp, eof_selected_track, i) == eof_note_type))
		{	//If the note is selected and in the active instrument difficulty
			next_note = eof_track_fixup_next_note(sp, track, i);	//Get the index of the next note in the active instrument difficulty
			if(amount)
			{	//If adjusted the note length by the specified number of ms
				if(dir < 0)
				{	//If the note length is to be decreased
					if(notelength < 2)
					{	//If the note cannot have its length decreased
						continue;	//Skip adjusting this note
					}
					if(!undo_made)
					{	//Ensure an undo state was made before decreasing the length
						eof_prepare_undo(EOF_UNDO_TYPE_NOTE_LENGTH);
						undo_made = 1;
					}
					eof_set_note_length(sp, eof_selected_track, i, notelength - adjustment);
				}
				else
				{	//If the note length is to be increased
					if(next_note > 0)
					{	//Check if the increase would be canceled due to being unable to overlap the next note
						if((notepos + notelength + eof_min_note_distance >= eof_get_note_pos(sp, track, next_note)) && !(eof_get_note_flags(sp, track, i) & EOF_NOTE_FLAG_CRAZY))
						{	//If this note cannot increase its length because it would overlap the next (taking the minimum note distance into account) and the note isn't "crazy"
							continue;	//Skip adjusting this note
						}
					}
					if(!undo_made)
					{	//Ensure an undo state was made before increasing the length
						eof_prepare_undo(EOF_UNDO_TYPE_NOTE_LENGTH);
						undo_made = 1;
					}
					eof_set_note_length(sp, eof_selected_track, i, notelength + adjustment);
				}
			}
			else
			{	//If adjusting by the current grid snap value
				unsigned long targetpos = notepos + notelength;	//The position from which the end of the note's tail will reposition
				long beat = eof_get_beat(sp, targetpos);		//Get the beat in which the tail currently ends
				unsigned long newtailendpos = targetpos;

				if((dir < 0) && (beat > 0) && (beat < sp->beats) && (targetpos == sp->beat[beat]->pos))
				{	//If the note is being shortened by a grid snap and the tail's current end position is found to be at the start of a beat
					targetpos--;	//Consider the tail's current position to end in the previous beat so that the grid snap logic will find it a grid snap position in that beat
				}
				eof_snap_logic(&eof_tail_snap, targetpos);	//Find grid snap positions before and after the tail's current ending position
				if(dir < 0)
				{	//If the tail is being shortened by one grid snap
					if(eof_tail_snap.previous_snap > notepos)
					{	//Only allow the tail to shorten to end at the previous grid snap if it will still end after the note begins
						newtailendpos = eof_tail_snap.previous_snap;
					}
					else
					{	//Otherwise enforce the minimum length of 1ms
						newtailendpos = notepos + 1;
					}
				}
				else
				{	//If the tail is being lengthened by one grid snap
					if(eof_tail_snap.next_snap > notepos + notelength)
					{	//If the next grid snap position was able to be determined
						if(next_note > 0)
						{	//Check if the increase would be canceled due to being unable to overlap the next note
							if((notepos + notelength + eof_min_note_distance >= eof_get_note_pos(sp, track, next_note)) && !(eof_get_note_flags(sp, track, i) & EOF_NOTE_FLAG_CRAZY))
							{	//If this note cannot increase its length because it would overlap the next (taking the minimum note distance into account) and the note isn't "crazy"
								continue;	//Skip adjusting this note
							}
						}
						newtailendpos = eof_tail_snap.next_snap;
					}
				}
				if(newtailendpos != notepos + notelength)
				{	//If the note length is actually changing
					if(!undo_made)
					{	//Ensure an undo state was made before increasing the length
						eof_prepare_undo(EOF_UNDO_TYPE_NOTE_LENGTH);
						undo_made = 1;
					}
					eof_note_set_tail_pos(sp, eof_selected_track, i, newtailendpos);
				}
				newnotelength = eof_get_note_length(sp, eof_selected_track, i);
				if(newnotelength > 1)
				{	//If the note's length, after the adjustment, is over 1
					eof_snap_logic(&eof_tail_snap, notepos + newnotelength);
					eof_note_set_tail_pos(sp, eof_selected_track, i, eof_tail_snap.pos);	//Clean up the grid snap increase/decrease by re-snapping the tail

					newnotelength2 = eof_get_note_length(sp, eof_selected_track, i);
					if((dir > 0) && (notelength == newnotelength2))
					{	//Special case:  If the grid snap length increase was nullified by the snap logic, force the tail to increase one snap interval higher
						double difference = eof_tail_snap.grid_pos[1] - eof_tail_snap.grid_pos[0];	//This is the length of one grid snap in the target beat

						eof_note_set_tail_pos(sp, eof_selected_track, i, notepos + notelength + difference + 0.5);	//Resnap the tail one grid snap higher
					}
					else if((dir < 0) && (notelength == newnotelength2))
					{	//Special case:  If a grid snap decrease did not shorten the note (ie. it began on a grid snap position)
						eof_snap_logic(&eof_tail_snap, notepos + notelength - 1);	//Find grid snap positions in the previous grid snap
						eof_note_set_tail_pos(sp, eof_selected_track, i, eof_tail_snap.previous_snap);
					}
				}
			}//If adjusting by the current grid snap value
			if(eof_get_note_length(sp, track, i) <= 0)
			{	//If the note's length became less than 1 for any reason
				eof_set_note_length(sp, track, i, 1);	//Set it to 1ms
			}
		}//If the note is selected and in the active instrument difficulty
	}//For each note in the track
	eof_track_fixup_notes(sp, eof_selected_track, 1);
	if(note_selection_updated)
	{	//If the only note modified was the seek hover note
		eof_selection.multi[eof_seek_hover_note] = 0;	//Deselect it to restore the note selection's original condition
		eof_selection.current = EOF_MAX_NOTES - 1;
	}
}

void eof_set_num_arpeggios(EOF_SONG *sp, unsigned long track, unsigned long number)
{
	unsigned long tracknum;

 	eof_log("eof_set_num_arpeggios() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
			sp->pro_guitar_track[tracknum]->arpeggios = number;
		break;
	}
}

void eof_track_delete_arpeggio(EOF_SONG *sp, unsigned long track, unsigned long index)
{
	unsigned long ctr;
	unsigned long tracknum;

 	eof_log("eof_track_delete_arpeggio() entered", 1);

	if((sp == NULL) || (track >= sp->tracks))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(index < sp->pro_guitar_track[tracknum]->arpeggios)
			{
				sp->pro_guitar_track[tracknum]->arpeggio[index].name[0] = '\0';	//Empty the name string
				for(ctr = index; ctr < sp->pro_guitar_track[tracknum]->arpeggios; ctr++)
				{
					memcpy(&sp->pro_guitar_track[tracknum]->arpeggio[ctr], &sp->pro_guitar_track[tracknum]->arpeggio[ctr + 1], sizeof(EOF_PHRASE_SECTION));
				}
				sp->pro_guitar_track[tracknum]->arpeggios--;
			}
		break;
	}
}

void eof_set_note_name(EOF_SONG *sp, unsigned long track, unsigned long note, char *name)
{
// 	eof_log("eof_set_note_name() entered");

	unsigned long tracknum;

	if((sp == NULL) || (track >= sp->tracks) || (name == NULL))
		return;
	tracknum = sp->track[track]->tracknum;

	switch(sp->track[track]->track_format)
	{
		case EOF_LEGACY_TRACK_FORMAT:
			if(note < sp->legacy_track[tracknum]->notes)
			{
				(void) ustrcpy(sp->legacy_track[tracknum]->note[note]->name, name);
			}
		break;

		case EOF_VOCAL_TRACK_FORMAT:
			if(note < sp->vocal_track[tracknum]->lyrics)
			{
				(void) ustrcpy(sp->vocal_track[tracknum]->lyric[note]->text, name);
			}
		break;

		case EOF_PRO_GUITAR_TRACK_FORMAT:
			if(note < sp->pro_guitar_track[tracknum]->notes)
			{
				(void) ustrcpy(sp->pro_guitar_track[tracknum]->note[note]->name, name);
			}
		break;
	}
}

int eof_detect_string_gem_conflicts(EOF_PRO_GUITAR_TRACK *tp, unsigned long newnumstrings)
{
	unsigned long ctr, bitmask, i, higheststring = 0, conflict = 0;

	if(tp == NULL)
		return -1;	//Return error

	for(i = tp->notes; i > 0; i--)
	{	//For each note in the track
		for(ctr=0, bitmask=1; ctr < 8; ctr++, bitmask <<= 1)
		{	//For each lane
			if(tp->note[i-1]->note & bitmask)
			{	//If this lane is populated
				if(ctr + 1 > higheststring)
				{
					higheststring = ctr + 1;	//Track the highest used string in the track (ctr counts beginning at zero)
				}
				if(ctr >= newnumstrings)
				{	//If the string count is higher than the specified string
					conflict = 1;	//Note that a conflict occurred
				}
			}
		}
	}

	if(conflict)
	{	//If there was a conflict
		return higheststring;	//Return the highest used string number
	}
	return 0;	//Return no conflict
}

int eof_get_pro_guitar_note_fret_string(EOF_PRO_GUITAR_TRACK *tp, unsigned long note, char *pro_guitar_string)
{
	unsigned long i, bitmask, index, fretvalue;

	if(!tp || (note >= tp->notes) || !pro_guitar_string)
	{	//If there was an invalid parameter
		return 0;	//Return error
	}

	for(i = 0, bitmask = 1, index = 0; i < tp->numstrings; i++, bitmask<<=1)
	{	//For each of the track's usable strings
		if(index != 0)
		{	//If another fret value was already written to this string
			pro_guitar_string[index++] = ' ';	//Insert a space
		}
		if(tp->note[note]->note & bitmask)
		{	//If the string is populated for the selected pro guitar note
			fretvalue = tp->note[note]->frets[i];
			if(fretvalue & 0x80)
			{	//If this string is muted (MSB set)
				pro_guitar_string[index++] = 'X';	//Write a capital x to indicate muted string
			}
			else
			{
				if(fretvalue > 9)
				{	//If the fret value uses two digits instead of one
					pro_guitar_string[index++] = '0' + (fretvalue / 10);	//Write the tens digit
				}
				pro_guitar_string[index++] = '0' + (fretvalue % 10);	//Write the ones digit
			}
		}
		else
		{
			pro_guitar_string[index++] = '_';	//Write an underscore to indicate string not played
		}
	}
	pro_guitar_string[index] = '\0';	//Terminate the string

	return 1;	//Return success
}

int eof_five_lane_drums_enabled(void)
{
	return (eof_song->track[EOF_TRACK_DRUM]->flags & EOF_TRACK_FLAG_SIX_LANES);
}

char eof_track_has_cymbals(EOF_SONG *sp, unsigned long track)
{
	unsigned long i, note, noteflags;

	if((sp == NULL) || (track >= sp->tracks))
		return 0;

	if(sp->track[track]->track_behavior == EOF_DRUM_TRACK_BEHAVIOR)
	{	//If this is a drum track
		for(i = 0; i < eof_get_track_size(sp, track); i++)
		{	//For each note in the track
			note = eof_get_note_note(sp, track, i);
			noteflags = eof_get_note_flags(sp, track, i);
			if(	((note & 4) && ((noteflags & EOF_DRUM_NOTE_FLAG_Y_CYMBAL))) ||
				((note & 8) && ((noteflags & EOF_DRUM_NOTE_FLAG_B_CYMBAL))) ||
				((note & 16) && ((noteflags & EOF_DRUM_NOTE_FLAG_G_CYMBAL))))
			{	//If this note contains a yellow, blue or green drum marked with pro drum notation
				return 1;	//Track has cymbals
			}
		}
	}

	return 0;	//Track has no cymbals
}

char eof_search_for_note_near(EOF_SONG *sp, unsigned long track, unsigned long targetpos, unsigned long delta, char type, unsigned long *match)
{
	unsigned long i, notepos, distance = 0;
	char matchfound = 0;

	if((sp == NULL) || (track >= sp->tracks) || (match == NULL))
		return 0;

	for(i = 0; i < eof_get_track_size(sp, track); i++)
	{	//For each note in the specified track
		if(eof_get_note_type(sp, track, i) == type)
		{	//If this note is in the specified difficulty
			notepos = eof_get_note_pos(sp, track, i);
			if((notepos < targetpos) && (targetpos - notepos <= delta))
			{	//If this note is before the target but in range
				if(!matchfound || (targetpos - notepos < distance))
				{	//If this note is the first match or is closer than the previous match
					distance = targetpos - notepos;
					*match = i;
					matchfound = 1;
				}
			}
			else if(notepos - targetpos <= delta)
			{	//If this note is after the target but in range
				if(!matchfound || (notepos - targetpos < distance))
				{	//If this note is the first match or is closer than the previous match
					distance = notepos - targetpos;
					*match = i;
					matchfound = 1;
				}
			}
		}
	}

	return matchfound;	//Return the match status
}

int eof_thin_notes_to_match_target_difficulty(EOF_SONG *sp, unsigned long sourcetrack, unsigned long targettrack, unsigned long delta, char type)
{
	unsigned long i, match;

	if((sp == NULL) || (sourcetrack >= sp->tracks) || (targettrack >= sp->tracks))
		return 0;

	eof_prepare_undo(EOF_UNDO_TYPE_NONE);
	for(i = eof_get_track_size(sp, targettrack); i > 0; i--)
	{	//For each note in the target track (in reverse order)
		if(eof_get_note_type(sp, targettrack, i - 1) == type)
		{	//If this note is in the specified difficulty
			if(!eof_search_for_note_near(sp, sourcetrack, eof_get_note_pos(sp, targettrack, i - 1), delta, type, &match))
			{	//If this note is not within the specified range of any note in the source track difficulty
				eof_track_delete_note(sp, targettrack, i - 1);	//Delete the note
			}
		}
	}

	return 1;
}

unsigned long eof_get_highest_fret(EOF_SONG *sp, unsigned long track, char scope)
{
	unsigned long highestfret = 0, currentfret, ctr, ctr2, tracknum, bitmask;
	int note_selection_updated;

	if(!sp || (track >= sp->tracks))
		return 0;	//Invalid parameters
	if(sp->track[track]->track_format != EOF_PRO_GUITAR_TRACK_FORMAT)
		return 0;	//Only run this on a pro guitar/bass track

	note_selection_updated = eof_feedback_mode_update_note_selection();	//If no notes are selected, select the seek hover note if Feedback input mode is in effect
	tracknum = sp->track[track]->tracknum;
	for(ctr = 0; ctr < sp->pro_guitar_track[tracknum]->notes; ctr++)
	{	//For each note in the specified pro guitar track
		if(!scope || ((eof_selection.track == track) && eof_selection.multi[ctr]))
		{	//If this note is within the scope of this search (in the track or selected)
			for(ctr2 = 0, bitmask = 1; ctr2 < 6; ctr2++, bitmask<<=1)
			{	//For each of the 6 usable strings
				if(sp->pro_guitar_track[tracknum]->note[ctr]->note & bitmask)
				{	//If this string is in use
					currentfret = sp->pro_guitar_track[tracknum]->note[ctr]->frets[ctr2];
					if((currentfret != 0xFF) && ((currentfret & 0x7F) > highestfret))
					{	//If this fret value (masking out the MSB, which is used for muting status) is higher than the previous
						highestfret = currentfret & 0x7F;
					}
				}
			}
		}
	}

	if(note_selection_updated)
	{	//If the only note modified was the seek hover note
		eof_selection.multi[eof_seek_hover_note] = 0;	//Deselect it to restore the note selection's original condition
		eof_selection.current = EOF_MAX_NOTES - 1;
	}
	return highestfret;
}

unsigned long eof_get_highest_clipboard_fret(char *clipboardfile)
{
	PACKFILE * fp;
	unsigned long sourcetrack = 0, copy_notes = 0;
	unsigned long i, j, bitmask;
	unsigned long highestfret = 0, currentfret;	//Used to find if any pasted notes would use a higher fret than the active track supports
	EOF_EXTENDED_NOTE temp_note = {{0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0, 0, 0, {0}, {0}, 0, 0, 0, 0, 0};

	if(!clipboardfile)
	{	//If the passed clipboard filename is invalid
		return 0;
	}
	fp = pack_fopen(clipboardfile, "r");
	if(!fp)
	{	//If the clipboard couldn't be opened
		return 0;
	}
	sourcetrack = pack_igetl(fp);	//Read the source track of the clipboard data
	copy_notes = pack_igetl(fp);	//Read the number of notes on the clipboard
	(void) pack_igetl(fp);					//Read the original beat number of the first note that was copied
	if(!copy_notes)
	{	//If there are 0 notes on the clipboard
		return 0;
	}
	if(eof_song->track[sourcetrack]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
	{	//If the clipboard notes are from a pro guitar/bass track
		for(i = 0; i < copy_notes; i++)
		{	//For each note in the clipboard file
			eof_read_clipboard_note(fp, &temp_note, EOF_NAME_LENGTH + 1);	//Read the note
			for(j = 0, bitmask = 1; j < 6; j++, bitmask <<= 1)
			{	//For each of the 6 usable strings
				if(temp_note.note & bitmask)
				{	//If this string is in use
					currentfret = temp_note.frets[j];
					if((currentfret != 0xFF) && ((currentfret & 0x7F) > highestfret))
					{	//If this fret value (masking out the MSB, which is used for muting status) is higher than the previous
						highestfret = currentfret & 0x7F;
					}
				}
			}
		}
	}
	(void) pack_fclose(fp);

	return highestfret;
}

unsigned long eof_get_highest_clipboard_lane(char *clipboardfile)
{
	PACKFILE * fp;
	unsigned long copy_notes = 0;
	unsigned long i, j, bitmask;
	unsigned long highestlane = 0;	//Used to find if any pasted notes would use a higher lane than the active track supports
	EOF_EXTENDED_NOTE temp_note = {{0}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.0, 0.0, 0, 0, 0, {0}, {0}, 0, 0, 0, 0, 0};

	if(!clipboardfile)
	{	//If the passed clipboard filename is invalid
		return 0;
	}
	fp = pack_fopen(clipboardfile, "r");
	if(!fp)
	{	//If the clipboard couldn't be opened
		return 0;
	}
	(void) pack_igetl(fp);			//Read the source track of the clipboard data
	copy_notes = pack_igetl(fp);	//Read the number of notes on the clipboard
	(void) pack_igetl(fp);			//Read the original beat number of the first note that was copied
	if(!copy_notes)
	{	//If there are 0 notes on the clipboard
		return 0;
	}
	for(i = 0; i < copy_notes; i++)
	{	//For each note in the clipboard file
		eof_read_clipboard_note(fp, &temp_note, EOF_NAME_LENGTH + 1);	//Read the note
		for(j = 1, bitmask = 1; j < 9; j++, bitmask<<=1)
		{	//For each of the 8 bits in the bitmask
			if(temp_note.note & bitmask)
			{	//If this bit is in use
				if(j > highestlane)
				{	//If this lane is higher than the previously tracked highest lane
					highestlane = j;
				}
			}
		}
	}
	(void) pack_fclose(fp);

	return highestlane;
}

unsigned long eof_get_lowest_fret_value(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long lowestfret = 0, currentfret, ctr, tracknum, bitmask;

	if((sp == NULL) || (track >= sp->tracks) || (sp->track[track]->track_format != EOF_PRO_GUITAR_TRACK_FORMAT))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;
	if(note >= sp->pro_guitar_track[tracknum]->notes)
		return 0;	//Return error

	for(ctr = 0, bitmask = 1; ctr < 6; ctr++, bitmask<<=1)
	{	//For each of the 6 usable strings
		if(sp->pro_guitar_track[tracknum]->note[note]->note & bitmask)
		{	//If this string is in use
			currentfret = sp->pro_guitar_track[tracknum]->note[note]->frets[ctr];
			if((currentfret != 0xFF) && (((currentfret & 0x7F) < lowestfret) || !lowestfret))
			{	//If this fret value (masking out the MSB, which is used for muting status) is lower than the previous lowest value, or no fret value has been stored
				lowestfret = currentfret & 0x7F;
			}
		}
	}

	return lowestfret;
}

unsigned long eof_get_highest_fret_value(EOF_SONG *sp, unsigned long track, unsigned long note)
{
	unsigned long highestfret = 0, currentfret, ctr, tracknum, bitmask;

	if((sp == NULL) || (track >= sp->tracks) || (sp->track[track]->track_format != EOF_PRO_GUITAR_TRACK_FORMAT))
		return 0;	//Return error
	tracknum = sp->track[track]->tracknum;
	if(note >= sp->pro_guitar_track[tracknum]->notes)
		return 0;	//Return error

	for(ctr = 0, bitmask = 1; ctr < 6; ctr++, bitmask<<=1)
	{	//For each of the 6 usable strings
		if(sp->pro_guitar_track[tracknum]->note[note]->note & bitmask)
		{	//If this string is in use
			currentfret = sp->pro_guitar_track[tracknum]->note[note]->frets[ctr];
			if((currentfret != 0xFF) && ((currentfret & 0x7F) > highestfret))
			{	//If this fret value (masking out the MSB, which is used for muting status) is higher than the previous
				highestfret = currentfret & 0x7F;
			}
		}
	}

	return highestfret;
}

unsigned long eof_determine_chart_length(EOF_SONG *sp)
{
	unsigned long lastitempos = 0;	//This will track the position of the last text event, or the end of the last note/lyric, whichever is later
	unsigned long ctr, ctr2, thisendpos, thiseventbeat;
	EOF_PRO_GUITAR_TRACK *tp = NULL;

	if(!sp)
		return 0;

	//Check notes
	for(ctr = 1; ctr < sp->tracks; ctr++)
	{	//For each track in the project
		if(sp->track[ctr]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
		{	//If this is a pro guitar track
			tp = sp->pro_guitar_track[sp->track[ctr]->tracknum];
			for(ctr2 = 0; ctr2 < tp->pgnotes; ctr2++)
			{	//For each normal note in the track
				thisendpos = tp->pgnote[ctr2]->pos + tp->pgnote[ctr2]->length;	//The end position of this note
				if(thisendpos > lastitempos)
				{
					lastitempos = thisendpos;	//Track the end position of the last note/lyric
				}
			}
			for(ctr2 = 0; ctr2 < tp->technotes; ctr2++)
			{	//For each technote in the track
				thisendpos = tp->technote[ctr2]->pos + tp->technote[ctr2]->length;	//The end position of this note
				if(thisendpos > lastitempos)
				{
					lastitempos = thisendpos;	//Track the end position of the last note/lyric
				}
			}
			for(ctr2 = 0; ctr2 < tp->notes; ctr2++)
			{	//For each note (either tech or normal, whichever is in effect, if the calling function didn't set pgnotes or technotes counters yet) in the track
				thisendpos = tp->note[ctr2]->pos + tp->note[ctr2]->length;	//The end position of this note
				if(thisendpos > lastitempos)
				{
					lastitempos = thisendpos;	//Track the end position of the last note/lyric
				}
			}
		}
		else
		{	//This is a non pro guitar track
			for(ctr2 = 0; ctr2 < eof_get_track_size(sp, ctr); ctr2++)
			{	//For each note/lyric in the track
				thisendpos = eof_get_note_pos(sp, ctr, ctr2) + eof_get_note_length(sp, ctr, ctr2);	//The end position of this note/lyric
				if(thisendpos > lastitempos)
				{
					lastitempos = thisendpos;	//Track the end position of the last note/lyric
				}
			}
		}
	}

	//Check text events
	for(ctr = 0; ctr < sp->text_events; ctr++)
	{	//For each text event in the project
		thiseventbeat = sp->text_event[ctr]->beat;
		if(thiseventbeat < sp->beats)
		{	//If this text event is on a valid beat marker
			thisendpos = sp->beat[thiseventbeat]->pos;
			if(thisendpos > lastitempos)
			{	//If this text event is later than all of the notes/lyrics in the project
				lastitempos = thisendpos;	//Track its position
			}
		}
	}

	//Check bookmarks
	for(ctr = 0; ctr < EOF_MAX_BOOKMARK_ENTRIES; ctr++)
	{	//For each bookmark
		if(sp->bookmark_pos[ctr] && (sp->bookmark_pos[ctr] > lastitempos))
		{	//If this bookmark is defined and it is later than the last note/lyric/text event
			lastitempos = sp->bookmark_pos[ctr];	//Track its position
		}
	}

	return lastitempos;
}

void eof_truncate_chart(EOF_SONG *sp)
{
	unsigned long ctr, targetpos, targetbeat;

	if(!sp || !sp->beats)
		return;

 	eof_log("eof_truncate_chart() entered", 1);

	targetpos = eof_determine_chart_length(sp);	//Find the chart native length

	//Determine the larger of the last item position and the end of the loaded audio
	if(eof_music_length > targetpos)
	{
		targetpos = eof_music_length;
	}

	if(sp->beats > 1)
	{	//As long as there are multiple beats
		targetpos += sp->beat[sp->beats - 1]->fpos - sp->beat[sp->beats - 2]->fpos + 0.5;	//Add an extra beat to the target position for padding
	}
	if(sp->beat[sp->beats - 1]->pos <= targetpos)
	{	//If there aren't enough beats so that at least one starts after the target position
		eof_log("\tAdding beats", 1);
		while(sp->beat[sp->beats - 1]->pos <= targetpos)
		{	//While there aren't enough beats so that at least one starts after the target position
			if(!eof_song_append_beats(sp, 1))
			{	//If there was an error adding a beat
				eof_log("\tError adding beat.  Aborting", 1);
				return;
			}
		}
		eof_chart_length = sp->beat[sp->beats - 1]->pos;	//The position of the last beat is the new chart length
	}
	else
	{	//Find the beat that precedes the target position
		double beat_length;

		eof_log("\tDeleting beats", 1);
		for(targetbeat = 0; targetbeat < sp->beats; targetbeat++)
		{	//For each beat
			if((targetbeat + 1 >= sp->beats) || (sp->beat[targetbeat + 1]->pos > targetpos))
			{	//If this is the last beat, or the next beat is after the target position
				break;
			}
		}
		beat_length = eof_calc_beat_length(sp, targetbeat);				//Get the length of the beat
		targetpos = sp->beat[targetbeat]->pos + beat_length + beat_length + 0.5;	//The chart length will be resized to last to the end of the last beat that has contents/audio, and another beat further for padding
		eof_chart_length = targetpos;	//Resize the chart length accordingly

		//Truncate empty beats
		for(ctr = sp->beats; ctr > 0; ctr--)
		{	//For each beat (in reverse order)
			if(sp->beat[ctr - 1]->pos > eof_chart_length)
			{	//If this beat is beyond the end of the populated chart and the audio
				eof_song_delete_beat(sp, ctr - 1);	//Remove it from the end of the chart
			}
		}
		if((sp == eof_song) && (eof_selected_beat >= sp->beats))
		{	//If the selected beat in the active project is no longer valid
			eof_selected_beat = 0;	//Select the first beat
		}
	}

 	eof_log("eof_truncate_chart() exiting", 1);
}

unsigned long eof_get_note_max_length(EOF_SONG *sp, unsigned long track, unsigned long note, char enforcegap)
{
	long next = note;
	unsigned long thisflags, thispos = 0, nextpos = 0;
	unsigned char thisnote, nextnote;
	int effective_min_note_distance = eof_min_note_distance;	//By default, the user configured minimum note distance is used

	if(!sp || (track >= sp->tracks))
		return 0;	//Return error

	thisflags = eof_get_note_flags(sp, track, note);	//Get the note's flags so it can be checked for "crazy" status
	thisnote = eof_get_note_note(sp, track, note);		//Also get its note bitflag
	if((eof_song->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT) && (thisflags & EOF_PRO_GUITAR_NOTE_FLAG_LINKNEXT))
	{	//If this is a pro guitar note and it has linknext status
		effective_min_note_distance = 0;	//The note is allowed to extend all the way up to the next note
	}
	while(1)
	{
		next = eof_track_fixup_next_note(sp, track, next);	//Find the next note that follows the specified note
		if(next < 0)
		{	//If there was no next note
			return ULONG_MAX;	//This note has no length limit
		}

		nextnote = eof_get_note_note(sp, track, next);	//Get the next note's note bitflag
		if(thisflags & EOF_NOTE_FLAG_CRAZY)
		{	//If this is a crazy note, it is not limited by the next note unless the next note has any lanes in common
			if((thisnote & nextnote) == 0)
			{	//If this note and the next have no lanes in common
				continue;	//Compare with the next note in another loop iteration instead
			}
		}

		thispos = eof_get_note_pos(sp, track, note);	//Get the note's position
		nextpos = eof_get_note_pos(sp, track, next);	//And the next note's
		if(nextpos - thispos <= effective_min_note_distance)
		{	//If the notes aren't far enough apart to enforce the minimum note distance
			return 1;
		}
		else
		{
			break;
		}
	}
	if(enforcegap)
	{	//If the minimum distance between notes is being observed
		return (nextpos - thispos - effective_min_note_distance);
	}
	else
	{
		return (nextpos - thispos);
	}
}

int eof_check_if_notes_exist_beyond_audio_end(EOF_SONG *sp)
{
	unsigned long ctr, ctr2;

	if(!sp || eof_silence_loaded)
	{	//If the specified chart is not valid or there is no chart audio loaded
		return 0;
	}

	for(ctr = 1; ctr < sp->tracks; ctr++)
	{	//For each track in the chart
		for(ctr2 = 0; ctr2 < eof_get_track_size(sp, ctr); ctr2++)
		{	//For each note in the chart
			if(eof_get_note_pos(sp, ctr, ctr2) + eof_get_note_length(sp, ctr, ctr2) > eof_music_length)
			{	//If the note ends beyond the chart audio
				return ctr;
			}
		}
	}

	return 0;
}

void eof_flatten_difficulties(EOF_SONG *sp, unsigned long srctrack, unsigned char srcdiff, unsigned long desttrack, unsigned char destdiff, unsigned long threshold)
{
	unsigned long ctr, ctr2, notecount, phrasecount, targetnote, targetpos;
	long targetlength;
	char undo_made = 0;

	if((sp == NULL) || (srctrack >= sp->tracks) || (desttrack >= sp->tracks))
		return;	//Invalid parameters

 	eof_log("eof_flatten_difficulties() entered", 1);

	//Flatten notes
	eof_track_sort_notes(sp, srctrack);				//This logic relies on notes being sorted by time and then by difficulty
	notecount = eof_get_track_size(sp, srctrack);	//Cache this value, since any new notes will be appended to the track
	eof_determine_phrase_status(sp, srctrack);		//Update the tremolo status of each note
	ctr = 0;
	while(ctr < notecount)
	{	//For each pre-existing note in the source track
		if(eof_get_note_type(sp, srctrack, ctr) <= srcdiff)
		{	//If this note is at or below the source difficulty
			targetnote = ctr;	//Track which note is to be copied to the source difficulty
			targetpos = eof_get_note_pos(sp, srctrack, ctr);	//Track this note's position
			targetlength = eof_get_note_length(sp, srctrack, ctr);	//This this note's length
			for(; ctr < notecount; ctr++)
			{	//For each of the remaining pre-existing notes in the track
				if(eof_get_note_pos(sp, srctrack, ctr) <= targetpos + threshold)
				{	//If this note is within the threshold of the note to be copied to the source difficulty
					targetnote = ctr;	//This note is higher in difficulty and is a more suitable note to copy to the destination difficulty
					targetlength = eof_get_note_length(sp, srctrack, ctr);
				}
				else
				{	//This note is too far away, consider it a different note
					break;
				}
			}
			if((eof_get_note_type(sp, srctrack, targetnote) != destdiff) || (srctrack != desttrack))
			{	//If the candidate note to be copied to the destination difficulty isn't already the note in the destination difficulty
				if(!undo_made)
				{	//If an undo state hasn't been made yet
					eof_prepare_undo(EOF_UNDO_TYPE_NONE);
					undo_made = 1;
				}
				(void) eof_copy_note(sp, srctrack, targetnote, desttrack, targetpos, targetlength, destdiff);	//Copy the note to the destination track difficulty
				if(eof_get_note_flags(sp, srctrack, targetnote) & EOF_NOTE_FLAG_IS_TREMOLO)
				{	//If the copied note has tremolo status
					targetpos = eof_get_note_pos(sp, srctrack, targetnote);
					(void) eof_track_add_section(sp, desttrack, EOF_TREMOLO_SECTION, destdiff, targetpos, targetpos + targetlength, 0, NULL);	//Add a tremolo phrase in the destination track difficulty
				}
			}
		}
	}
	eof_track_sort_notes(sp, desttrack);
	(void) eof_detect_difficulties(eof_song, desttrack);

	//Flatten fret hand positions
	if(sp->track[srctrack]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
	{	//If this is a pro guitar/bass track
		EOF_PRO_GUITAR_TRACK *tp, *dtp;
		EOF_PHRASE_SECTION *ptr;

		tp = sp->pro_guitar_track[sp->track[srctrack]->tracknum];	//Pointer to source track
		dtp = sp->pro_guitar_track[sp->track[desttrack]->tracknum];	//Pointer to destination track
		phrasecount = tp->handpositions;
		for(ctr = 0; ctr < phrasecount; ctr++)
		{	//For each pre-existing fret hand position in the source track
			if(tp->handposition[ctr].difficulty <= srcdiff)
			{	//If this hand position is at or below the target difficulty
				if(!undo_made)
				{	//If an undo state hasn't been made yet
					eof_prepare_undo(EOF_UNDO_TYPE_NONE);
					undo_made = 1;
				}
				ptr = eof_pro_guitar_track_find_effective_fret_hand_position_definition(dtp, destdiff, tp->handposition[ctr].start_pos, NULL, NULL, 1);
				if(ptr)
				{	//If there is already a fret hand position at this position in the destination track difficulty
					ptr->end_pos = dtp->handposition[ctr].end_pos;	//Update the existing fret hand position entry (end_pos is the fret number)
				}
				else
				{	//Otherwise add the fret hand to the destination track difficulty by copying it from the source track
					(void) eof_track_add_section(sp, desttrack, EOF_FRET_HAND_POS_SECTION, destdiff, tp->handposition[ctr].start_pos, tp->handposition[ctr].end_pos, 0, NULL);
					eof_pro_guitar_track_sort_fret_hand_positions(dtp);	//Sort the positions
				}
			}
		}
		//Remove consecutive hand positions using the same fret number
		for(ctr = dtp->handpositions; ctr > 0; ctr--)
		{	//For each fret hand position in the destination track, in reverse order
			if(ctr > 1)
			{	//If there's a fret hand position before this one
				if(dtp->handposition[ctr - 2].difficulty == dtp->handposition[ctr - 1].difficulty)
				{	//And it's in the same difficulty
					if(dtp->handposition[ctr - 2].end_pos == dtp->handposition[ctr - 1].end_pos)
					{	//And has the same fret number
						if(!undo_made)
						{	//If an undo state hasn't been made yet
							eof_prepare_undo(EOF_UNDO_TYPE_NONE);
							undo_made = 1;
						}
						eof_pro_guitar_track_delete_hand_position(dtp, ctr - 1);	//Delete this fret hand position and keep the earlier one
					}
				}
			}
		}

	//Flatten arpeggio sections
		phrasecount = tp->arpeggios;
		for(ctr = 0; ctr < phrasecount; ctr++)
		{	//For each pre-existing arpeggio in the source track
			if(tp->arpeggio[ctr].difficulty <= srcdiff)
			{	//If this arpeggio is at or below the target difficulty, determine if this arpeggio overlaps with one already added to the destination track difficulty
				ptr = NULL;
				for(ctr2 = 0; ctr2 < dtp->arpeggios; ctr2++)
				{	//For each arpeggio in the destination track
					if(dtp->arpeggio[ctr].difficulty == destdiff)
					{	//If the arpeggio is in the destination track difficulty
						if((tp->arpeggio[ctr].start_pos <= dtp->arpeggio[ctr].end_pos) && (tp->arpeggio[ctr].end_pos >= dtp->arpeggio[ctr].start_pos))
						{	//If the arpeggio section in the source track and the destination track difficulty overlap
							ptr = &dtp->arpeggio[ctr];	//Store this arpeggio's address
							break;
						}
					}
				}
				if(!undo_made)
				{	//If an undo state hasn't been made yet
					eof_prepare_undo(EOF_UNDO_TYPE_NONE);
					undo_made = 1;
				}
				if(ptr)
				{	//If there is already an arpeggio at this position in the destination track difficulty
					if(tp->arpeggio[ctr].start_pos < ptr->start_pos)
					{	//Update the starting position to the earlier of the arpeggios
						ptr->start_pos = tp->arpeggio[ctr].start_pos;
					}
					if(tp->arpeggio[ctr].end_pos > ptr->end_pos)
					{	//Update the ending position to the later of the arpeggios
						ptr->end_pos = tp->arpeggio[ctr].end_pos;
					}
				}
				else
				{	//Otherwise add the arpeggio to the destination track difficulty by copying it from the source track
					(void) eof_track_add_section(sp, desttrack, EOF_ARPEGGIO_SECTION, destdiff, tp->arpeggio[ctr].start_pos, tp->arpeggio[ctr].end_pos, 0, NULL);
					tp->arpeggio[tp->arpeggios - 1].flags = tp->arpeggio[ctr].flags;	//Copy the arpeggio flags in case it was a handshape that was copied
				}
			}
		}//For each pre-existing arpeggio in the source track
		eof_track_fixup_notes(sp, desttrack, 1);	//Run cleanup logic to create base chords for arpeggio/handshape phrases if applicable
	}//If this is a pro guitar/bass track
}

void eof_track_add_or_remove_track_difficulty_content_range(EOF_SONG *sp, unsigned long track, unsigned char diff, unsigned long startpos, unsigned long endpos, int function, int prompt, char *undo_made)
{
	unsigned long ctr, ctr2, ctr3;
	EOF_PRO_GUITAR_TRACK *tp = NULL;
	EOF_PHRASE_SECTION *ptr;
	int notearrayctr = 1;	//If the target track is a pro guitar track, the note alteration logic will be run for both the normal AND tech note arrays
	char restore_tech_view = 0;		//If tech view is in effect, it is temporarily disabled until after the secondary piano roll has been rendered

	if(!sp || !undo_made || (track >= sp->tracks) || (startpos >= endpos))
		return;	//Invalid parameters

	if(eof_song->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
	{	//If the track being altered is a pro guitar track
		tp = eof_song->pro_guitar_track[eof_song->track[track]->tracknum];
		notearrayctr = 2;	//A second note array (the tech notes) will need to be processed
		restore_tech_view = eof_menu_track_get_tech_view_state(sp, track);
		eof_menu_track_set_tech_view_state(sp, track, 0);	//Disable tech view if applicable
	}

	for(ctr3 = 0; ctr3 < notearrayctr; ctr3++)
	{	//For each note array being modified
		if(tp && (ctr3 > 0))
		{	//If this is the second pass, the tech notes are being modified
			eof_menu_pro_guitar_track_enable_tech_view(tp);
		}

		//Parse in reverse order so that notes can be appended to or deleted from the track in the same loop
		for(ctr = eof_get_track_size(sp, track); ctr > 0; ctr--)
		{	//For each note in the track (in reverse order)
			unsigned long notepos = eof_get_note_pos(sp, track, ctr - 1);
			unsigned char notetype = eof_get_note_type(sp, track, ctr - 1);

			//Prompt to fix notes that are aligned slightly outside of a phrase
			if((prompt != 3) && (notepos < startpos) && (notepos + 10 >= startpos))
			{	//If this note is 1 to 10 milliseconds before the beginning of the affected time range
				if(!prompt)
				{	//If the user wasn't prompted about how to handle this condition yet, seek to the note in question and prompt the user whether to take action
					eof_seek_and_render_position(eof_selected_track, eof_note_type, notepos);
					eof_clear_input();
					if(alert("At least one note is between 1 and 10 ms before the phrase.", NULL, "Move such notes to the start of the phrase?", "&Yes", "&No", 'y', 'n') == 1)
					{	//If the user opts to correct the note positions
						prompt = 1;	//Store a "yes" response
					}
					else
					{
						prompt = 2;	//Store a "no" response
					}
				}
				if(prompt == 1)
				{	//If the user opted to correct the note positions
					if(!*undo_made)
					{	//If an undo state hasn't been made yet
						eof_prepare_undo(EOF_UNDO_TYPE_NONE);
						*undo_made = 1;
					}
					for(ctr2 = eof_get_track_size(sp, track); ctr2 > 0; ctr2--)
					{	//For each note in the track (in reverse order)
						unsigned long notepos2 = eof_get_note_pos(sp, track, ctr2 - 1);
						if((notepos2 < startpos) && (notepos2 + 10 >= startpos))
						{	//If this note is 1 to 10 milliseconds before the beginning of the phrase
							eof_set_note_pos(sp, track, ctr2 - 1, startpos);	//Re-position the note to the start of the phrase
						}
					}
					notepos = startpos;	//Update the stored position for the note
				}
			}

			//Update notes
			if((notetype >= diff) && (notepos >= startpos) && (notepos <= endpos))
			{	//If the note meets the criteria to be altered
				if(!*undo_made)
				{	//If an undo state hasn't been made yet
					eof_prepare_undo(EOF_UNDO_TYPE_NONE);
					*undo_made = 1;
				}
				if(notetype == diff)
				{	//If this note is in the target difficulty
					if(function < 0)
					{	//If the delete level function is being performed, this note will be deleted
						eof_track_delete_note(sp, track, ctr - 1);
					}
					else
					{	//The add level function is being performed, this note will be duplicated into the next higher difficulty instead of just having its difficulty incremented
						long length = eof_get_note_length(sp, track, ctr - 1);
						(void) eof_copy_note(sp, track, ctr - 1, track, notepos, length, diff + 1);
					}
				}
				else
				{	//This note is in a higher difficulty (the outer if statement filters out notes in lower difficulties)
					if(function < 0)
					{	//If the delete level function is being performed, this note will have its difficulty decremented
						eof_set_note_type(sp, track, ctr - 1, notetype - 1);	//Decrement the note's difficulty
					}
					else
					{	//The add level function is being performed, this note will have its difficulty incremented
						eof_set_note_type(sp, track, ctr - 1, notetype + 1);	//Increment the note's difficulty
					}
				}
			}
		}//For each note in the track (in reverse order)
		eof_track_sort_notes(sp, track);	//Sort the note array
	}//For each note array being modified
	eof_menu_track_set_tech_view_state(sp, track, restore_tech_view);	//Re-enable tech view if applicable

	if(tp)
	{	//If the track being altered is a pro guitar track
		//Update arpeggios
		for(ctr = eof_get_num_arpeggios(sp, track); ctr > 0; ctr--)
		{	//For each arpeggio section in the track (in reverse order)
			ptr = &tp->arpeggio[ctr - 1];	//Simplify
			if((ptr->difficulty >= diff) && (ptr->start_pos <= endpos) && (ptr->end_pos >= startpos))
			{	//If this arpeggio overlaps at all with the phrase being manipulated and is in a difficulty level that this function affects
				if(ptr->difficulty == diff)
				{	//If this arpeggio is in the active difficulty
					if(function < 0)
					{	//If the delete level function is being performed, this arpeggio will be deleted
						eof_track_delete_arpeggio(sp, track, ctr - 1);
					}
					else
					{	//The add level function is being performed, this arpeggio will be duplicated into the next higher difficulty instead of just having its difficulty incremented
						EOF_PHRASE_SECTION *newptr;
						(void) eof_track_add_section(sp, track, EOF_ARPEGGIO_SECTION, diff + 1, ptr->start_pos, ptr->end_pos, 0, NULL);
						newptr = &tp->arpeggio[tp->arpeggios - 1];	//Pointer to the new arpeggio phrase
						newptr->flags = ptr->flags;	//Copy the arpeggio flags in case it was a handshape that was copied
					}
				}
				else
				{
					if(function < 0)
					{	//If the delete level function is being performed, this arpeggio will have its difficulty decremented
						ptr->difficulty--;	//Decrement the arpeggio's difficulty
					}
					else
					{	//The add level function is being performed, this arpeggio will have its difficulty incremented
						ptr->difficulty++;	//Increment the arpeggio's difficulty
					}
				}
			}
		}

		//Update fret hand positions
		for(ctr = tp->handpositions; ctr > 0; ctr--)
		{	//For each fret hand position in the track (in reverse order)
			ptr = &tp->handposition[ctr - 1];	//Simplify
			if((ptr->difficulty >= diff) && (ptr->start_pos >= startpos) && (ptr->start_pos <= endpos))
			{	//If this fret hand position is within the phrase being manipulated and is in a difficulty level that this function affects
				if(ptr->difficulty == diff)
				{	//If this fret hand position is in the active difficulty, it will be duplicated into the next higher difficulty instead of just having its difficulty incremented
					if(function < 0)
					{	//If the delete level function is being performed, this fret hand position will be deleted
						eof_pro_guitar_track_delete_hand_position(tp, ctr - 1);
					}
					else
					{	//The add level function is being performed, this fret hand position will be duplicated into the next higher difficulty instead of just having its difficulty incremented
						(void) eof_track_add_section(sp, track, EOF_FRET_HAND_POS_SECTION, diff + 1, ptr->start_pos, ptr->end_pos, 0, NULL);
					}
				}
				else
				{
					if(function < 0)
					{	//If the delete level function is being performed, this fret hand position will have its difficulty decremented
						ptr->difficulty--;	//Decrement the fret hand position's difficulty
					}
					else
					{	//The add level function is being performed, this fret hand position will have its difficulty incremented
						ptr->difficulty++;	//Increment the fret hand position's difficulty
					}
				}
			}
		}
		eof_pro_guitar_track_sort_fret_hand_positions(tp);	//Resort fret hand positions

		//Update tremolos
		for(ctr = eof_get_num_tremolos(sp, track); ctr > 0; ctr--)
		{	//For each tremolo section in the track (in reverse order)
			ptr = &tp->tremolo[ctr - 1];	//Simplify
			if((ptr->difficulty >= diff) && (ptr->start_pos <= endpos) && (ptr->end_pos >= startpos))
			{	//If this tremolo overlaps at all with the phrase being manipulated and is in a difficulty level that this function affects
				if(ptr->difficulty == 0xFF)
				{	//If this tremolo is not difficulty specific
					continue;	//Don't manipulate it in this function
				}
				if(ptr->difficulty == diff)
				{	//If this tremolo is in the active difficulty
					if(function < 0)
					{	//If the delete level function is being performed, this tremolo will be deleted
						eof_track_delete_tremolo(sp, track, ctr - 1);
					}
					else
					{	//The add level function is being performed, this tremolo will be duplicated into the next higher difficulty instead of just having its difficulty incremented
						(void) eof_track_add_section(sp, track, EOF_TREMOLO_SECTION, diff + 1, ptr->start_pos, ptr->end_pos, 0, NULL);
					}
				}
				else
				{
					if(function < 0)
					{	//If the delete level function is being performed, this tremolo will have its difficulty decremented
						ptr->difficulty--;	//Decrement the tremolo's difficulty
					}
					else
					{	//The add level function is being performed, this tremolo will have its difficulty incremented
						ptr->difficulty++;	//Increment the tremolo's difficulty
					}
				}
			}
		}
	}//If the track being altered is a pro guitar track
}

void eof_unflatten_difficulties(EOF_SONG *sp, unsigned long track)
{
	unsigned long beatctr, startpos = 0, endpos = 0;
	unsigned char diffctr;
	int prompt = 0;	//Only the first call to eof_track_add_or_remove_track_difficulty_content_range() for each phrase will check/prompt to re-align notes
	char undo_made = 0, started = 0;

	if((sp == NULL) || (track >= sp->tracks))
		return;	//Invalid parameters

 	eof_log("eof_unflatten_difficulties() entered", 1);

	for(beatctr = 0; beatctr < sp->beats; beatctr++)
	{	//For each beat in the project
		if((sp->beat[beatctr]->contained_section_event >= 0) || ((beatctr + 1 >= sp->beats) && started))
		{	//If this beat has a section event (RS phrase) or a phrase is in progress and this is the last beat, it marks the end of any current phrase and the potential start of another
			if(started)
			{	//If the first phrase marker has been encountered, this beat marks the end of a phrase
				endpos = sp->beat[beatctr]->pos - 1;	//Track this as the end position of the phrase

				//Remove the higher of two difficulties' identical phrases
				for(diffctr = 255; diffctr > 0; diffctr--)
				{	//For each difficulty, in reverse order
					if(eof_track_diff_populated_status[diffctr])
					{	//If this difficulty is populated
						if(!eof_compare_time_range_with_previous_or_next_difficulty(sp, track, startpos, endpos, diffctr, -1))
						{	//If the notes of this phrase are the same between this difficulty and the previous
							eof_track_add_or_remove_track_difficulty_content_range(sp, track, diffctr, startpos, endpos, -1, prompt, &undo_made);	//Level down the content of this time range of the track difficulty
							prompt = 3;	//Ensure that subsequent calls to the above function don't check note alignment for this phrase
						}
					}
				}
			}//If the first phrase marker has been encountered, this beat marks the end of a phrase

			started = 1;	//Track that a phrase has been encountered
			prompt = 0;		//The next call to eof_track_add_or_remove_track_difficulty_content_range() will check note alignment for the beginning of this phrase
			startpos = eof_song->beat[beatctr]->pos;	//Track the starting position of the phrase
		}//If this beat has a section event (RS phrase) or a phrase is in progress and this is the last beat, it marks the end of any current phrase and the potential start of another
	}//For each beat in the project

	sp->track[track]->numdiffs = eof_detect_difficulties(sp, track);	//Rebuild the eof_track_diff_populated_status[] array, and update the track's difficulty count
	if(eof_note_type >= sp->track[track]->numdiffs)
	{	//If the active track difficulty is now invalid
		eof_note_type = sp->track[track]->numdiffs - 1;	//Change to the current highest difficulty
	}
}

void eof_erase_track_content(EOF_SONG *sp, unsigned long track, unsigned char diff, char diffonly)
{
	unsigned long i, tracknum;
	EOF_PHRASE_SECTION *ptr;
	EOF_PRO_GUITAR_TRACK *tp = NULL;
	char restore_tech_view;		//If tech view is in effect, it is temporarily disabled until after the secondary piano roll has been rendered

	if(!sp || (track >= sp->tracks))
		return;	//Invalid parameters

	tracknum = sp->track[track]->tracknum;
	restore_tech_view = eof_menu_track_get_tech_view_state(sp, track);
	eof_menu_track_set_tech_view_state(sp, track, 0);	//Disable tech view if applicable

	//Delete notes
	for(i = eof_get_track_size(sp, track); i > 0; i--)
	{	//For each note/lyric in the track, in reverse order
		if(!diffonly || (eof_get_note_type(sp, track, i - 1) == diff))
		{	//If the entire track is to be erased, or if this note is in the difficulty specified to be erased
			eof_track_delete_note(sp, track, i - 1);	//Delete it
		}
	}

	//Delete arpeggios
	for(i = eof_get_num_arpeggios(sp, track); i > 0; i--)
	{	//For each arpeggio phrase in the track, in reverse order
		ptr = eof_get_arpeggio(sp, track, i - 1);
		if(ptr)
		{	//If this phrase could be found
			if(!diffonly || (ptr->difficulty == diff))
			{	//If the entire track is to be erased, or if this arpeggio is in the difficulty specified to be erased
				eof_track_delete_arpeggio(sp, track, i - 1);	//Delete it
			}
		}
	}

	//Delete tech notes, popup messages, hand positions and tones
	if(sp->track[track]->track_format == EOF_PRO_GUITAR_TRACK_FORMAT)
	{	//If a pro guitar track was specified
		//Delete tech notes
		tp = eof_song->pro_guitar_track[tracknum];
		eof_menu_pro_guitar_track_enable_tech_view(tp);
		for(i = eof_get_track_size(sp, track); i > 0; i--)
		{	//For each tech note in the track, in reverse order
			if(!diffonly || (eof_get_note_type(sp, track, i - 1) == diff))
			{	//If the entire track is to be erased, or if this note is in the difficulty specified to be erased
				eof_track_delete_note(sp, track, i - 1);	//Delete it
			}
		}
		eof_menu_track_set_tech_view_state(sp, track, restore_tech_view);	//Re-enable tech view if applicable
		if(!diffonly)
		{	//If the entire track is to be erased
			tp->popupmessages = 0;	//Remove all of the track's popup messages
			tp->tonechanges = 0;	//Remove all of the track's tone changes
		}
		for(i = tp->handpositions; i > 0; i--)
		{	//For each of the track's fret hand positions, in reverse order
			if(!diffonly || (tp->handposition[i - 1].difficulty == diff))
			{	//If the entire track is to be erased, or if this hand position is in the difficulty specified to be erased
				eof_pro_guitar_track_delete_hand_position(tp, i - 1);	//Delete it
			}
		}
		eof_pro_guitar_track_sort_fret_hand_positions(tp);	//Sort the positions, since they must be in order for displaying to the user
	}

	//Delete tremolos
	for(i = eof_get_num_tremolos(sp, track); i > 0; i--)
	{	//For each tremolo phrase in the track, in reverse order
		ptr = eof_get_tremolo(sp, track, i - 1);
		if(ptr)
		{	//If this phrase could be found
			if(!diffonly || (ptr->difficulty == diff))
			{	//If the entire track is to be erased, or if this tremolo is in the difficulty specified to be erased
				eof_track_delete_tremolo(sp, track, i - 1);	//Delete it
			}
		}
	}

	//Delete phrases, etc.
	if(!diffonly)
	{	//If the entire track is to be erased
		eof_set_num_solos(sp, track, 0);
		eof_set_num_star_power_paths(sp, track, 0);
		eof_set_num_trills(sp, track, 0);
		eof_set_num_sliders(sp, track, 0);

		if(sp->track[track]->track_format == EOF_VOCAL_TRACK_FORMAT)
		{
			sp->vocal_track[tracknum]->lines = 0;
		}
	}
}

void eof_erase_track(EOF_SONG *sp, unsigned long track)
{
	eof_erase_track_content(sp, track, 0, 0);
}

void eof_erase_track_difficulty(EOF_SONG *sp, unsigned long track, unsigned char diff)
{
	eof_erase_track_content(sp, track, diff, 1);
}

void eof_hightlight_all_notes_above_fret_number(EOF_SONG *sp, unsigned long track, unsigned char fretnum)
{
	unsigned long ctr, tracknum;

	if(!sp || (track >= sp->tracks))
		return;	//Invalid parameters
	if(sp->track[track]->track_format != EOF_PRO_GUITAR_TRACK_FORMAT)
		return;	//Only run this on a pro guitar/bass track

	tracknum = sp->track[track]->tracknum;
	for(ctr = 0; ctr < sp->pro_guitar_track[tracknum]->notes; ctr++)
	{	//For each note in the specified pro guitar track
		if(eof_get_highest_fret_value(sp, track, ctr) > fretnum)
		{	//If the note uses a fret number higher than the specified fret
			sp->pro_guitar_track[tracknum]->note[ctr]->flags |= EOF_NOTE_FLAG_HIGHLIGHT;	//Set the note's highlight flag
		}
	}
}

void eof_track_remove_highlighting(EOF_SONG *sp, unsigned long track, char function)
{
	unsigned long ctr, flags;

	if(!sp || (track >= sp->tracks))
		return;	//Invalid parameters

	for(ctr = 0; ctr < eof_get_track_size(sp, track); ctr++)
	{	//For each note in the specified track
		if(!function || (function > 1))
		{	//If the calling function signaled to remove the permanent highlight flag
			flags = eof_get_note_flags(sp, track, ctr);
			flags &= ~EOF_NOTE_FLAG_HIGHLIGHT;
			eof_set_note_flags(sp, track, ctr, flags);	//Clear the note's permanent hightlight flag
		}
		if((function == 1) || (function > 1))
		{	//If the calling function signaled to remove the temporary highlight flag
			flags = eof_get_note_tflags(sp, track, ctr);
			flags &= ~EOF_NOTE_TFLAG_HIGHLIGHT;
			eof_set_note_tflags(sp, track, ctr, flags);	//Clear the note's temporary hightlight flag
		}
	}
}

SAMPLE *eof_export_time_range_sample = NULL;
unsigned long eof_export_time_range_ctr;

static void eof_export_time_range_callback(void * buffer, int nsamples, int stereo)
{
	int i;
	int iter = stereo ? 2 : 1;
	unsigned long index;
	unsigned short *in_buffer = (unsigned short *)buffer;
	unsigned short *out_buffer = (unsigned short *)eof_export_time_range_sample->data;

	for(i = 0; i < nsamples / iter; i++)
	{	//For each decoded sample
		index = i * iter;
		out_buffer[eof_export_time_range_ctr + index] = in_buffer[index];	//Copy amplitude to output buffer
		if(stereo)
		{
			out_buffer[eof_export_time_range_ctr + index + 1] = in_buffer[index + 1];	//Copy the other channel's amplitude to output buffer
		}
	}
	eof_export_time_range_ctr += nsamples;
}

void eof_export_time_range(ALOGG_OGG * ogg, double start_time, double end_time, const char * fn)
{
	unsigned long num_samples;

	if(!ogg || !fn)
		return;	//Invalid parameters

	num_samples = (end_time - start_time) * alogg_get_wave_freq_ogg(ogg) * (alogg_get_wave_is_stereo_ogg(ogg) ? 2 : 1);
	eof_export_time_range_sample = create_sample(alogg_get_wave_bits_ogg(ogg), alogg_get_wave_is_stereo_ogg(ogg), alogg_get_wave_freq_ogg(ogg), num_samples / (alogg_get_wave_is_stereo_ogg(ogg) ? 2 : 1));
	eof_export_time_range_ctr = 0;

	if(eof_export_time_range_sample)
	{	//If memory for the decoded audio was allocated
		if(alogg_process_ogg(ogg, eof_export_time_range_callback, 1024, start_time, end_time))
		{	//Successfully decoded audio file
			(void) save_wav(fn, eof_export_time_range_sample);	//Export it to WAV
		}
		destroy_sample(eof_export_time_range_sample);
		eof_export_time_range_sample = NULL;
	}
}

char eof_pro_guitar_note_has_tech_note(EOF_PRO_GUITAR_TRACK *tp, unsigned long note, unsigned long *technote_num)
{
	if((tp == NULL) || (note >= tp->notes))
		return 0;	//Return error

	return eof_pro_guitar_note_bitmask_has_tech_note(tp, note, tp->note[note]->note, technote_num);	//Search for a technote on any of this note's used strings
}

char eof_pro_guitar_note_bitmask_has_tech_note(EOF_PRO_GUITAR_TRACK *tp, unsigned long note, unsigned long mask, unsigned long *technote_num)
{
	unsigned long ctr, notepos, notelen;
	long nextnote;

	if((tp == NULL) || (note >= tp->notes))
		return 0;	//Return error

	notepos = tp->pgnote[note]->pos;
	notelen = tp->pgnote[note]->length;
	nextnote = eof_fixup_next_pro_guitar_note(tp, note);
	if(nextnote > 0)
	{	//If there was a next note
		if(notepos + notelen == tp->pgnote[nextnote]->pos)
		{	//And this note extends all the way to it with no gap in between (this note has linkNext status)
			notelen--;	//Shorten the effective note length to ensure that a tech note at the next note's position is detected as affecting that note instead of this one
		}
	}
	for(ctr = 0; ctr < tp->technotes; ctr++)
	{	//For each tech note in the track
		if(tp->technote[ctr]->pos > notepos + notelen)
		{	//If this tech note (and all those that follow) are after the end position of this note
			break;	//Break from loop, no overlapping note will be found
		}
		if((tp->technote[ctr]->type == tp->pgnote[note]->type) && (mask & tp->technote[ctr]->note))
		{	//If the tech note is in the same difficulty as the pro guitar regular note and uses any of the specified strings
			if((tp->technote[ctr]->pos >= notepos) && (tp->technote[ctr]->pos <= notepos + notelen))
			{	//If this tech note overlaps with the specified pro guitar regular note
				if(technote_num)
				{	//If the calling function passed a non NULL address
					*technote_num = ctr;	//Return the index number of the first tech note that overlaps with the specified note
				}
				return 1;	//Return overlapping tech note found
			}
		}
	}
	return 0;	//Return overlapping tech note not found
}

unsigned long eof_pro_guitar_note_bitmask_has_bend_tech_note(EOF_PRO_GUITAR_TRACK *tp, unsigned long note, unsigned long mask, unsigned long *technote_num)
{
	unsigned long ctr, flags, notepos, notelen, matchctr = 0;
	long nextnote;

	if((tp == NULL) || (note >= tp->notes))
		return 0;	//Return error

	notepos = tp->pgnote[note]->pos;
	notelen = tp->pgnote[note]->length;
	nextnote = eof_fixup_next_pro_guitar_note(tp, note);
	if(nextnote > 0)
	{	//If there was a next note
		if(notepos + notelen == tp->pgnote[nextnote]->pos)
		{	//And this note extends all the way to it with no gap in between (this note has linkNext status)
			notelen--;	//Shorten the effective note length to ensure that a tech note at the next note's position is detected as affecting that note instead of this one
		}
	}
	for(ctr = 0; ctr < tp->technotes; ctr++)
	{	//For each tech note in the track
		if(tp->technote[ctr]->pos > notepos + notelen)
		{	//If this tech note (and all those that follow) are after the end position of this note
			break;	//Break from loop, no overlapping note will be found
		}
		if((tp->technote[ctr]->type == tp->pgnote[note]->type) && (mask & tp->technote[ctr]->note))
		{	//If the tech note is in the same difficulty as the pro guitar regular note and uses any of the specified strings
			if((tp->technote[ctr]->pos >= notepos) && (tp->technote[ctr]->pos <= notepos + notelen))
			{	//If this tech note overlaps with the specified pro guitar regular note
				flags = tp->technote[ctr]->flags;
				if((flags & EOF_PRO_GUITAR_NOTE_FLAG_BEND) && (flags & EOF_PRO_GUITAR_NOTE_FLAG_RS_NOTATION))
				{	//If the tech note is a bend and has a bend strength defined
					if(technote_num && !matchctr)
					{	//If the calling function passed a non NULL address, and this is the first overlapping bend tech note found
						*technote_num = ctr;	//Store the index number of the first tech note that overlaps with the specified note
					}
					matchctr++;	//Increment counter
				}
			}
		}
	}
	return matchctr;	//Return the number of overlapping bend notes found
}

char eof_pro_guitar_tech_note_overlaps_a_note(EOF_PRO_GUITAR_TRACK *tp, unsigned long technote, unsigned long mask, unsigned long *note_num)
{
	unsigned long ctr, techpos;
	long nextnote, notelen;
	EOF_PRO_GUITAR_NOTE *np;
	char restore_tech_view = 0;		//If tech view is in effect, it is temporarily disabled

	if((tp == NULL) || (technote >= tp->technotes))
		return 0;	//Return error

	restore_tech_view = eof_menu_pro_guitar_track_get_tech_view_state(tp);
	eof_menu_pro_guitar_track_disable_tech_view(tp);	//Disable tech view if applicable
	techpos = tp->technote[technote]->pos;
	for(ctr = 0; ctr < tp->pgnotes; ctr++)
	{	//For each regular note in the track
		np = tp->pgnote[ctr];	//Simplify
		notelen = np->length;	//Store the effective length for the note
		nextnote = eof_fixup_next_pro_guitar_note(tp, ctr);
		if(nextnote > 0)
		{	//If there was a next note
			if(np->pos + notelen == tp->pgnote[nextnote]->pos)
			{	//And this note extends all the way to it with no gap in between (this note has linkNext status)
				notelen--;	//Shorten the effective note length to ensure that a tech note at the next note's position is detected as affecting that note instead of this one
			}
		}

		if(np->pos > techpos)
		{	//If this regular note (and all those that follow) are after this tech note's position
			break;		//Break from loop
		}
		if((tp->technote[technote]->type == np->type) && (mask & np->note))
		{	//If the pro guitar note is in the same difficulty as the tech note and uses any of the specified strings
			if(np->pos == techpos)
			{	//If this regular note is at the same timestamp as the tech note
				if(note_num)
				{	//If the calling function passed a non NULL pointer
					*note_num = ctr;	//Pass the overlapping note number by reference
				}
				eof_menu_pro_guitar_track_set_tech_view_state(tp, restore_tech_view);	//Re-enable tech view if applicable
				return 1;	//Return overlap found at start of note
			}
			if((techpos >= np->pos) && (techpos <= np->pos + notelen))
			{	//If the tech note overlaps this regular note
				if(note_num)
				{	//If the calling function passed a non NULL pointer
					*note_num = ctr;	//Pass the overlapping note number by reference
				}
				eof_menu_pro_guitar_track_set_tech_view_state(tp, restore_tech_view);	//Re-enable tech view if applicable
				return 2;	//Return overlap found within a note
			}
		}
	}

	eof_menu_pro_guitar_track_set_tech_view_state(tp, restore_tech_view);	//Re-enable tech view if applicable
	return 0;	//No overlap note found
}

void eof_pro_guitar_track_enforce_chord_density(EOF_PRO_GUITAR_TRACK *tp)
{
	unsigned long ctr, threshold = 2;

	if(!tp)
		return;	//Invalid parameter

	if(tp->notes < 2)
		return;	//Don't run unless there are at least two notes in the track

	if(eof_min_note_distance > threshold)
	{	//If the configured minimum note distance is larger than the default threshold
		threshold = eof_min_note_distance;
	}
	for(ctr = 0; ctr < tp->notes - 1; ctr++)
	{	//For each note in the track from the first to the penultimate
		if((eof_selection.track == eof_selected_track) && eof_selection.multi[ctr] && (tp->note[ctr]->type == eof_note_type))
			continue;	//Skip this note if it is selected, as the user may wish to edit it before this function alters the next note

		if(eof_note_count_colors_bitmask(tp->note[ctr]->note) > 1)
		{	//If this note is a chord
			if(eof_pro_guitar_note_compare(tp, ctr, tp, ctr + 1, 0) == 0)
			{	//If the next note matches this chord
				if(tp->note[ctr + 1]->pos - tp->note[ctr]->pos - tp->note[ctr]->length > threshold)
				{	//If the space between these two chords is greater than the threshold distance
					tp->note[ctr + 1]->flags |= EOF_NOTE_FLAG_CRAZY;	//Apply the crazy flag to the next chord to force it to be low density during RS export
				}
			}
		}
	}
}

void eof_song_enforce_mid_beat_tempo_change_removal(void)
{
	unsigned long ctr;

	if(!eof_song)
		return;

	for(ctr = eof_song->beats; ctr > 0; ctr--)
	{	//For each beat (in reverse order)
		if(eof_song->beat[ctr - 1]->flags & EOF_BEAT_FLAG_MIDBEAT)
		{	//If this beat was flagged as a mid-beat tempo change during an import
			eof_song->beat[ctr - 1]->flags &= ~EOF_BEAT_FLAG_MIDBEAT;	//Clear the flag
			if(eof_db_import_drop_mid_beat_tempos)
			{	//If the user set the preference to delete such tempo changes
				eof_menu_beat_delete_logic(ctr - 1);
			}
		}
	}
}
