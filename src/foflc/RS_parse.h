#ifndef _rs_parse_h_
#define _rs_parse_h_

void Export_RS(FILE *outf);
	//Exports the loaded lyric structure to output file in Rocksmith XML format
void RS_Load(FILE *inf);
	//Perform all code necessary to load a Rocksmith format lyric file

int rs_filter_char(char character, char rs_filter);
	//Returns nonzero if character is any of the following characters:  ( } ,  \  : { " )
	//Returns nonzero if the character isn't an ASCII character (ie. greater than 127) or otherwise isn't a printable character
	//If rs_filter is greater than 1, the forward slash character is also not copied to the buffer
	//These characters can cause Rocksmith to crash if they are present in various free-text fields like chord names, lyric text or phrase names
	//Zero is returned if the character passed is not any of the offending characters
int rs_filter_string(char *string, char rs_filter);
	//Returns 1 if any character in the provided string is considered a filtered character by rs_filter_char()
	//Returns -1 on error
void expand_xml_text(char *buffer, size_t size, const char *input, size_t warnsize, char rs_filter);
	//Copies the input string into the specified buffer of the given size.  Any of the characters that XML requires to be escaped
	//are converted into the appropriate character sequence (ie. ' becomes &apos;).  If the expanded string's length is longer
	//than the given warning value, the user is given a warning message that the string will need to be shortened and the string
	//is truncated to be warnsize number of characters.  If warnsize is zero, no check or truncation is performed.
	//If size is zero, the function returns without doing anything.  Otherwise the buffer is guaranteed to be NULL terminated
	//If warnsize is larger than size, the function returns without doing anything.
	//If rs_filter is nonzero, the following characters are not copied to the buffer:  ( } ,  \  : { " ), and all non ASCII characters are likewise removed
	//If rs_filter is greater than 1, the forward slash character is also not copied to the buffer

void shrink_xml_text(char *buffer, size_t size, char *input);
	//Does the reverse of expand_xml_text(), converting each escape sequence into the appropriate individual character.
	//If size is zero, the function returns without doing anything.  Otherwise the buffer is guaranteed to be NULL terminated

int parse_xml_attribute_text(char *buffer, size_t size, char *target, char *input);
	//Parses the value of the specified target attribute from the specified input string into the given buffer
	//No more than size bytes are written to the buffer, which is guaranteed to be NULL terminated
	//Returns 1 on success

int parse_xml_attribute_number(char *target, char *input, long *output);
	//Parses and returns the numerical value of the specified target attribute from the specified input string
	//The numerical value is returned through *output
	//Returns 1 on success

int parse_xml_rs_timestamp(char *target, char *input, long *output);
	//Parses and returns the numerical value of the specified target attribute timestamp from the specified input string
	//Periods are discarded (which would convert Rocksmith formatted timestamps to milliseconds) and the value of the timestamp is returned through *output
	//Returns 1 on success


#endif //#ifndef _rs_parse_h_
