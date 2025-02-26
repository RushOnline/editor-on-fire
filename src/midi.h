#ifndef EOF_MIDI_H
#define EOF_MIDI_H

#include "song.h"
#include "foflc/Midi_parse.h"

#define EOF_MAX_TS 500
#define EOF_MAX_KS 100
#define EOF_MAX_MIDI_EVENTS (65536 * 2)

//Rock Band 3 Pro drum markers (force as drum instead of cymbal)
#define RB3_DRUM_YELLOW_FORCE 110	//Force the 2nd pad as a tom
#define RB3_DRUM_BLUE_FORCE 111		//Force the 3rd pad as a tom
#define RB3_DRUM_GREEN_FORCE 112	//Force the 4th pad as a tom

typedef struct
{
	unsigned long pos;
	int type;
	int note;
	int velocity;
	int channel;
	char * dp;
	char allocation;		//This tracks whether dp points to allocated that should be freed after it is written to file
	char filtered;			//This is set to nonzero by eof_check_for_note_overlap() if the event should be dropped instead of being written to MIDI (ie. overlaps another of the same MIDI note)
	char on;				//Simplifies the use of running status by indicating if this is a note on event
	char off;				//Simplifies the use of running status by indicating if this is a note off event
	unsigned long index;	//For text events, a tie in the sorting order will be broken by whichever has the lower index number
} EOF_MIDI_EVENT;


typedef struct
{
	unsigned long pos;		//Absolute delta time of this TS change
	double realtime;		//Realtime of this TS change
	unsigned int num;		//The numerator of the TS change (number of beats per measure)
	unsigned int den;		//The denominator of the TS change (note value representing one beat)
	unsigned long track;	//The track number containing this change
} EOF_MIDI_TS_CHANGE;

typedef struct
{
	EOF_MIDI_TS_CHANGE * change[EOF_MAX_TS];	//The list of time signatures changes
	unsigned long changes;					//The number of time signatures changes found in this list
} EOF_MIDI_TS_LIST;

typedef struct
{
	unsigned long pos;		//Absolute delta time of this TS change
	double realtime;		//Realtime of this TS change
	char key;			//Key signature
} EOF_MIDI_KS_CHANGE;

typedef struct
{
	EOF_MIDI_KS_CHANGE * change[EOF_MAX_KS];	//The list of key signatures changes
	unsigned long changes;					//The number of key signatures changes found in this list
} EOF_MIDI_KS_LIST;

int eof_export_midi(EOF_SONG * sp, char * fn, char featurerestriction, char fixvoxpitches, char fixvoxphrases, char format);
	//Writes the specified chart's contents to the specified file
	//If featurerestriction is 0, all chart features are written to MIDI, and expert+.mid is written if double bass drum is charted
	//If featurerestriction is 1, only RBN2 features are written to MIDI
	//If featurerestriction is 2, only pro guitar upgrade features are written to MIDI
	//If featurerestriction is 3, both RBN2 features and pro guitar features are written to MIDI.  The C3 release of Magma supports pro guitar/bass.
	//If format is nonzero, a variation of the MIDI is exported that is meant to be converted to .chart format and used to create GHWT customs:
	//	*Star power markers are written with MIDI note 106
	//	*Lane 6 drum notes (where lane 1 is the bass drum) that aren't cymbals are written with MIDI note 105
	//	*Open notes are written as 5 lane notes
	//	*Slider note markers are written with MIDI note 103 and solo phrases are suppressed
	///	*Accent drum notes are exported with track-specific text events defining accented lane numbers (numbered beginning with 1 instead of 0)
	//If fixvoxpitches is nonzero, any lyric that has a pitch of 0 (undefined) will be written with a generic pitch and a freestyle # marker
	//If fixvoxphrases is nonzero, any lyric that is not within a lyric phrase will have a phrase written to contain it

unsigned char eof_get_midi_pitches(EOF_SONG *sp, unsigned long track, unsigned long note, unsigned char *pitches);
	//Returns a bitmask defining which elements in the pitches array are populated to define the pitches used by the specified note/lyric
	//Gems that are string muted or ghosted are ignored
	//Each pitch is returned through *pitches array, which must be at least 6 elements large
	//0 is returned on error or if the specified note contains no pitches (pitchless or percussion lyric, or fully string muted or fully ghosted pro guitar note)
int eof_export_music_midi(EOF_SONG *sp, char *fn, char format);
	//Writes a normal MIDI file of the specified chart's vocal and pro guitar tracks to the specified file
	//For lyrics, each pitched lyric (vocal percussion notes are excluded) is written as a MIDI note on channel 0
	//For pro guitar notes, each non muted note of each used string is written as a MIDI note
	//If format is zero, a format suitable for Synthesia or Songs2See is written, where each note within an arrangement uses the same channel, incrementing from 1
	//If format is nonzero, a format suitable for Fretlight M-Player is written, where low E uses channel 15 and high E uses channel 10 (16 - lane number),
	//  all velocities are written as 127 and track names are prefixed with "FMP - " as required by the Fretlight M-Player program.
	//  In this format, each pro guitar/bass track is written twice (once each in Synthesia and Fretlight formats) to allow synth playback of arrangements in Fretlight M-Player

struct Tempo_change *eof_build_tempo_list(EOF_SONG *sp);
	//Parses the chart, returning a linked list of anchors (tempo changes), or NULL on error
	//Each link in the list gives the absolute deltatime and absolute realtime of the anchor

struct Tempo_change *eof_add_to_tempo_list(unsigned long delta, double realtime, double BPM, struct Tempo_change *ptr);
	//Creates and adds a new link EOF's tempo linked list referred to by ptr, and ptr is returned
	//If ptr is NULL, a new list is created and the new head pointer is returned
	//NULL is returned upon error

void eof_destroy_tempo_list(struct Tempo_change *ptr);
	//If ptr is not NULL, de-allocates all memory consumed by the tempo linked list

unsigned long eof_ConvertToDeltaTime(double realtime, struct Tempo_change *anchorlist, EOF_MIDI_TS_LIST *tslist, unsigned long timedivision, char snaptobeat);
	//An adaptation of the ConvertToDeltaTime() function from the FoFLC source
	//Parses a linked list of anchors and returns the delta time of the specified realtime
	//tslist is allowed to be NULL, but anchorlist must contain at least one tempo change
	//If snaptobeat is nonzero, the converted delta time is allowed to be adjusted +/- 1 delta to line up with a beat marker (use for start times, but not lengths or end times)

int eof_extract_rba_midi(const char * source, const char * dest);
	//Extracts the MIDI file embedded in the source RBA file and writes it to a regular MIDI file called dest
	//Returns nonzero on error

EOF_MIDI_TS_LIST * eof_create_ts_list(void);
	//Allocates and returns a TS change list
void eof_destroy_ts_list(EOF_MIDI_TS_LIST *ptr);
	//Deallocates the specified TS change list
void eof_midi_add_ts_deltas(EOF_MIDI_TS_LIST * changes, unsigned long pos, unsigned long num, unsigned long den, unsigned long track);
	//Adds the time signature information to the specified list of time signature changes, providing the absolute delta time of the TS change
void eof_midi_add_ts_realtime(EOF_MIDI_TS_LIST * changes, double pos, unsigned long num, unsigned long den, unsigned long track);
	//Adds the time signature information to the specified list of time signature changes, providing the absolute real time of the TS change
EOF_MIDI_TS_LIST *eof_build_ts_list(EOF_SONG *sp);
	//Parses sp->beat[], returning a list of time signature changes, or NULL on error
int eof_get_ts(EOF_SONG *sp, unsigned *num, unsigned *den, int beatnum);
	//If the specified beat number has a defined TS, return the num and den through the passed pointers if they are not NULL
	//Returns 1 if a time signature was returned
	//Returns 0 if the specified beat was not a time signature
	//Returns -1 on error
	//If 1 is not returned, the values referenced by numerator and denominator are not altered
int eof_get_effective_ts(EOF_SONG *sp, unsigned *num, unsigned *den, int beatnum);
	//Get the time signature in effect at the specified beat, return the num and den through the passed pointers if they are not NULL
	//If no time signature is found to be in effect, 4/4 is assumed
	//Returns 1 on success
	//Returns -1 on error
	//If 1 is not returned, the values referenced by numerator and denominator are not altered
int eof_apply_ts(unsigned num, unsigned den, int beatnum, EOF_SONG *sp, char undo);
	//Validates and applies the specified time signature to the specified beat
	//If undo is nonzero, then an undo state is made before any changes are made

EOF_MIDI_KS_LIST * eof_create_ks_list(void);
	//Allocates and returns a KS change list
EOF_MIDI_KS_LIST *eof_build_ks_list(EOF_SONG *sp);
	//Parses sp->beat[], returning a list of key signature changes, or NULL on error
void eof_destroy_ks_list(EOF_MIDI_KS_LIST *ptr);
	//Deallocates the specified KS change list

int eof_dump_midi_track(const char *inputfile, PACKFILE *outf);
	//Writes a MIDI track header to the output file, followed by the size of the input file, followed by the contents of the input file
void eof_write_text_event(unsigned long deltas, const char *str, PACKFILE *fp);
	//Writes a text event to the given file stream
void eof_write_lyric_event(unsigned long deltas, const char *str, PACKFILE *fp);
	//Writes a lyric event to the given file stream
void eof_write_tempo_track(char *trackname, struct Tempo_change *anchorlist, EOF_MIDI_TS_LIST *tslist, EOF_MIDI_KS_LIST *kslist, PACKFILE *outf);
	//Writes a tempo track to the given file stream, including all set tempo, set time signature and set key signature events and an end of track event
	//If trackname is not NULL, a track name MIDI event is written accordingly

void eof_add_sysex_event(unsigned long pos, int size, void *data);
	//Stores a copy of the Sysex message data (used for custom phrase markers in Phase Shift) to eof_midi_event[]
void eof_MIDI_data_track_export(EOF_SONG *sp, PACKFILE *outf, struct Tempo_change *anchorlist, EOF_MIDI_TS_LIST *tslist, unsigned long timedivision);
	//Write all stored MIDI track data from sp->midi_data_head to the output file
int eof_build_tempo_and_ts_lists(EOF_SONG *sp, struct Tempo_change **anchorlistptr, EOF_MIDI_TS_LIST **tslistptr, unsigned long *timedivision);
	//Accepts an EOF_SONG pointer, and builds the anchor and TS change lists, returning them through the pointers
	//If the passed chart contains a stored tempo track, that track is used to build the lists
	//Otherwise the chart's native tempo map is used to build them
	//If the passed chart contains a stored tempo track, that track's time division is returned via reference, otherwise EOF's default is returned via reference
	//Nonzero is returned upon success
void eof_check_vocals(EOF_SONG* sp, char *fixvoxpitches, char *fixvoxphrases);
	//Scans the vocal tracks contained in the chart for missing pitches or vocal phrases
	//If either are found, the user is prompted about whether to correct those problems during MIDI export
	//Nonzero is stored into either variable if the user opts for the correction
	//Zero is stored into either if the problem is not detected or the user declines
	//These user decisions will be passed to eof_export_midi()
void eof_check_for_note_overlap(void);
	//Checks the eof_midi_event[] array for overlapping note on/off events and filters them out as necessary
	//When two note on/off event pairs overlap, the innermost on and off events are filtered, allowing phrases to appropriately mark notes without overlapping
	//The events are expected to be sorted in chronological order
void eof_check_for_hopo_phrase_overlap(void);
	//Checks for any HOPO phrase that ends at the same delta position as a HOPO phrase of the opposite type (ie. a HOPO off phrase ending when a HOPO on phrase starts)
	//Any such phrase is altered to end one delta tick earlier, so that the two phrases are one delta apart from each other
	//This can happen in very special circumstances (ie. A hopo off note that overlaps a hopo on note and a hopo off note at the same time)

void eof_add_midi_event(unsigned long pos, int type, int note, int velocity, int channel);
	//Creates a new structure to store the specified values and appends it to eof_midi_event[]
	//Note on/off events are also tracked in the eof_midi_note_status[] array
void eof_add_midi_lyric_event(unsigned long pos, char * text, char allocation);
	//Creates a new structure to store the specified lyric event and appends it to eof_midi_event[]
	//The allocation boolean value specifies whether the string is using dynamically allocated memory and should be freed when the array is emptied
void eof_add_midi_text_event(unsigned long pos, char * text, char allocation, unsigned long index);
	//Creates a new structure to store the specified text event and appends it to eof_midi_event[]
	//The index parameter should be the event's index in the project, as it is used to resolve ties in MIDI event sorting
	//The allocation boolean value specifies whether the string is using dynamically allocated memory and should be freed when the array is emptied
void eof_clear_midi_events(void);
	//Frees the memory for all structures in eof_midi_event[], including any dynamically allocated strings
	//Resets the eof_midi_event_full variable to zero
void WriteVarLen(unsigned long value, PACKFILE * fp);
	//Writes the specified value in variable length format to the specified file handle
int qsort_helper3(const void * e1, const void * e2);
	//A sort algorithm used when quick sorting the eof_midi_event[] array

void eof_write_ghwt_drum_animations(EOF_SONG *sp, char *fn);
	//Writes a text file containing timestamps and codes for the specified chart, for use with authoring drum animations for Guitar Hero World Tour

#endif
