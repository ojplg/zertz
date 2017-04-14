#include "recording.h"

#include "board.h"
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>

/* x,y: [0..WIDTH-1] */

#define COL_LETTERS_MAX    6
static char col_letters[] = {'a','b','c','d','e','f','g'};

#define INTEGER_MAX(a,b) ((a>b ? a : b))




extern const char*
get_disc_notation( int x, int y, char *buffer, size_t maxlen )
{
	int letter_idx, rank;

	if (maxlen < 3)  return buffer; /* DUH */

	rank = 7 - INTEGER_MAX(x,y);
	letter_idx = x + (3 - y);

	if (rank < 0 || rank > WIDTH ||
		 letter_idx < 0 || letter_idx > COL_LETTERS_MAX) {
		/* XXX this is not good.  It might think
		 * there was an error but really there 
		 * was no disc to remove.
		 */ 
		printf("ARGH, bad coords\n");
		snprintf(buffer, 3, "--");

	} else {
		snprintf(buffer, 3, "%c%1d", col_letters[letter_idx], rank);
	}
	return buffer;

} /* get_disc_notation */


extern char
translate_marble_color(int color)
{
	switch (color) {
	case BLACK:
		return 'B';
	case GREY:
		return 'G';
	case WHITE:
		return 'W';    
	}
	return '?'; /* error */
} /* translate_marble_color */


/* open the requested file, initialize the record_stream */
extern record_stream*
open_record_file(const char *path) {

	record_stream* s;
	FILE* fstream;
	time_t t;

	/* mode="w": create or truncate */
	if ( (fstream = fopen(path, "w")) == NULL ) {
		fprintf(stderr, "Cannot create record file '%s'.\n", path);
		return NULL;
	}

	s = (record_stream*)malloc( sizeof(record_stream) );
	s->stream = fstream;
	s->move_number = 1;
	s->started_move = 0;

	/* ick.  ctime return value ends in \n */
	t = time(NULL);
	fprintf(s->stream, "#\n");
	fprintf(s->stream, "# Zertz game record\n");
	fprintf(s->stream, "# Game started %s", ctime(&t));
	fprintf(s->stream, "#\n");

	return s;

} /* open_record_file */


/* takes a ptrptr, so we can null the pointer for safety.
 * i bet that's hideously nonstandard. */
extern int
close_record_file( record_stream **s )
{
	int retval;
	record_stream *rs = *s; /* dereference the ptrptr */
	time_t t = time(NULL);

	if (rs==NULL) {return -1;} /* already closed */

	fprintf(rs->stream, "\n# Game ended %s", ctime(&t));
	/* clean up */
	retval = fclose(rs->stream);
	if( retval != 0 ){
		perror("recording: that is odd! fclose failed");
	}
	rs->stream = NULL; /* safe?  hmm. */
	free(rs);
	rs = NULL; /* zero the pointer to the stream so it can't be reused */
	return retval;

} /* close_record_file */


extern int
get_record_file_name(char *dir_path, int maxtries, char *buffer, size_t maxlen)
{
	int test_index;
	struct stat sinfo;

	/* start trying with zertz-1.rec, stop at zertz-$maxtries.rec */
	for ( test_index=1; test_index<=maxtries; test_index++ ) {
		if ( snprintf(buffer, maxlen, "%s/zertz-%d.rec", 
						  dir_path, test_index) < 0 ) {
			*buffer=0; /* set to null string */
			return -1;
		}
		if ( stat(buffer, &sinfo) < 0 ) {
			/* Can't stat: this probably means that a file of that name doesn't
			 * exist, and thus buffer contains an acceptable record filename[1].
			 * However, it could also mean that the directory doesn't exist
			 * or the uid doesn't have correct perms to read the dir, both of
			 * which can be caught during the call to fopen(),
			 * and anyway shouldn't happen if dir_path is chosen properly.
			 * (There are a few even less likely reasons, none of which should 
			 * be incompatible with fopen() failing.)
			 *
			 * [1] Of course, nothing prevents some other process from creating
			 *     this file after this check but before *you* get around to
			 *     doing anything with it.
			 */
			break; /* stop trying filenames */
		}
	}
	if (test_index>maxtries) { /* gave up, and name in buffer exists */
		*buffer=0;
		return -1;
	}
	/* buffer contains pseudo-"unique" namey goodness */
	return 0;

} /* get_record_file_name */


extern void
record_move_place( record_stream *s, int color, int column, int row )
{
	char scratch[3];
	if (!s->started_move) {
		s->started_move = 1;
		fprintf( s->stream, "%2d ", s->move_number );
	}
	fprintf( s->stream, "%c%s,",
				translate_marble_color(color),
				get_disc_notation(column,row, scratch,3) );
}

extern void
record_move_removal( record_stream *s, int column, int row )
{
	char scratch[3];
	fprintf(s->stream, "%s ", get_disc_notation(column,row, scratch,3));
}

extern void
record_move_start_capture( record_stream *s )
{
	if (!s->started_move) {
		s->started_move = 1;
		fprintf( s->stream, "%2d ", s->move_number );
	}
	fprintf( s->stream, "x ");
}

extern void
record_move_capture_step( record_stream *s, int col, int row, int cap_color )
{
	char scratch[3];
	fprintf( s->stream, "%s %c ",
				get_disc_notation(col,row, scratch,3),
				translate_marble_color(cap_color) );
}

extern void
record_move_capture_isolate( record_stream *s, int color, int col, int row )
{
	char scratch[3];
	fprintf( s->stream, "%c%s ",
				translate_marble_color(color),
				get_disc_notation(col,row, scratch,3) );
}

extern void
record_move_end_capture( record_stream *s, int column, int row )
{
	char scratch[3];
	fprintf(s->stream, "%s", get_disc_notation(column,row, scratch,3));
}

extern void
record_move_end( record_stream *s )
{
	fprintf(s->stream, "\n");
	fflush(s->stream); /* this is only to help tail -f watchers */
	/* reset s state for next move */
	s->move_number++;
	s->started_move = 0;
}

extern void
record_annotation( record_stream *s, const char *msg )
{
	fprintf(s->stream, msg);
}


/* ...set emacs options for proper style (keep this at the end of the file)...
 *
 * Local Variables:
 * c-tab-always-indent: nil
 * indent-tabs-mode: t
 * c-basic-offset: 3
 * tab-width: 3
 * End:
 *
 */

