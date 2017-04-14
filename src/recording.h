#ifndef __RECORDING_H
#define __RECORDING_H

#include <stdio.h>

typedef struct {
  FILE *stream;
  int move_number;
  int started_move; /* true unless last fn called was _move_end */
} record_stream;

/* Given a dir location for record files, generates a filename which
 * (at least at the moment this function is called) does not already
 * exist.  Generated filenames will be of the form 
 * "zertz-1.rec", "zertz-2.rec", ...
 *
 * dir_path: Path to directory in which to put record file.
 * maxtries: The hightest index this function will try to use in filename.
 * buffer: Generated filename will be stuffed here.  Should probably
 *         be big enough to hold PATH_MAX (from limits.h).
 * maxlen: Size of buffer.
 *
 * Returns: <0 on error.  An error retval indicates that no filename
 *          could be figured out, and buffer will be a zero-length string.
 *          Note that a successful return does not mean an open() on 
 *          the generated filename will succeed.
 */
extern int
get_record_file_name(char *dir_path, int maxtries, char *buffer,size_t maxlen);

/* Basic housekeeping: */

/* Open a record stream for the given filename. */
extern record_stream* open_record_file(const char *path);
/* Close record file, clean up, set *s to NULL. */
extern int close_record_file(record_stream **s);

/* Helper functions, also suitable for logging */
extern char translate_marble_color(int color);
extern const char* 
get_disc_notation (int x, int y, char *buffer, size_t maxlen);


/* Functions to write the actual game record: */

/*
 * There are two kinds of move notation: placement moves (consisting
 * of a marble drop and a ring removal (which may then result in
 * isolation captures)), and capture moves (consisting of a sequence
 * of jumps).
 *
 * A placement move is recorded thus:
 *   record_move_place()
 *   record_move_removal()
 * If the removal results in one or more captures by isolation,
 * then you should do a
 *   record_move_start_capture()
 * followed by the appropriate number of
 *   record_move_capture_isolate()
 *
 * A capture move is recorded thus:
 *   record_move_start_capture()
 *   record_move_capture_step()
 *    ...[ 0 or move additional capture steps ]...
 *   record_move_end_capture()
 *
 * A player's turn should be noted with record_move_end().
 */

/* The first half of a "normal" move notation: placing a marble. */
extern void record_move_place( record_stream *s, int color, int column, int row );
/* This is the second half of a "normal" move notation: the removed disc. */
extern void record_move_removal( record_stream *s, int column, int row );

/* This begins a capture notation, which can consist of many individual
 * jumps (steps). */
extern void record_move_start_capture( record_stream *s );
/* This is a half-step in a capture notation.  col/row indicate the
 * position the jumping marble is jumping from, and color indicates the
 * captured marble. */
extern void
record_move_capture_step( record_stream *s, int col, int row, int cap_color );
/* This records a capture due to isolation: col/row and color refer to
 * the captured marble, since no jumping is involved. */
extern void
record_move_capture_isolate( record_stream *s, int color, int col, int row );
/* This is the last step in a capture notation.  col/row indicate the
 * final location of the jumping marble. */
extern void record_move_end_capture( record_stream *s, int column, int row );

/* "Officially" end a move notation line. */
extern void record_move_end(record_stream *s);

/* Insert a random comment into the game record.
 * you should only do this if the last record_*() thing you called was
 * record_move_end().
 */
extern void record_annotation( record_stream *s, const char *msg );

#endif
