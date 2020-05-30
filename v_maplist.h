#ifndef V_MAPLIST_H
#define V_MAPLIST_H

#define MAX_VOTERS			(MAX_CLIENTS * 2)
#define MAX_MAPS			64
#define MAX_MAPNAME_LEN		16

#define MAPMODE_PVP			1
#define MAPMODE_PVM			2
#define MAPMODE_DOM			3
#define MAPMODE_CTF			4
#define MAPMODE_FFA			5
#define MAPMODE_INV			6

#define ML_ROTATE_SEQ          0
#define ML_ROTATE_RANDOM       1
#define ML_ROTATE_NUM_CHOICES  2

//***************************************************************************************
//***************************************************************************************

typedef struct v_maplist_s
{ 
   int  nummaps;          // number of maps in list 
   char mapnames[MAX_MAPS][MAX_MAPNAME_LEN];
} v_maplist_t;

//***************************************************************************************
//***************************************************************************************

typedef struct votes_s
{
	qboolean	used; // Paril: in "new" system, this means in progress.
	int			mode;
	int			mapindex;
	char		name[24];
	char		ip[16];
} votes_t;

//***************************************************************************************
//***************************************************************************************

#endif