/*------------------------------------------------------------------
 * fsm.h - Finite State Machine definitions
 *
 * February 2005, Bo Berry
 *
 * Copyright (c) 2005-2009 by Cisco Systems, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *------------------------------------------------------------------
 */

/*
    cfr: http://www.drdobbs.com/embedded-systems/the-embedded-finite-state-machine/217400611
         http://sourceforge.net/projects/efsm/

    Modificata con:
        *) funzioni di ingresso e uscita stato
        *) l'evento ha un parametro solo
        *) file di configurazione fsm_cfg.h con:
        	*) impostazioni
			*) malloc/free
			*) debug

	Esempio
		Elenco degli eventi
			typedef enum {
				evn1 = 0,
				evn2
			} E_VENTO ;

			static const event_description_t desc_evn[] = {
				{ evn1, "evento 1" },
				{ evn2, "evento 2" },
				{ evn3, NULL },			!!! evito la stampa !!!
				{ FSM_NULL_EVENT_ID, NULL }
			} ;

		Elenco degli stati
			typedef enum {
				stt1 = 0,
				stt2
			} S_TATO ;

			static const state_description_t desc_stt[] = {
				{ stt1, "Stato 1" },
				{ stt2, "Stato 2" },
				{ FSM_NULL_STATE_ID, NULL }
			} ;

		Definizione degli stati
			Ogni stato deve avere una tabella che associa ad ogni evento la
			funzione che lo gestisce

			static RC_FSM_t stt1_evn2(void * prm)
			{
				...

				se devo passare a uno stato diverso da quello della tabella:
					fsm_set_exception_state(fsm, stt1)

				return
					il valore viene restituito da fsm_engine
						RC_FSM_STOP_PROCESSING   : la macchina e' in distruzione (non evolve e non registra)
						RC_FSM_OK			     : si passa allo stato della tabella
												   o a quello di fsm_set_exception_state
						!= -> RC_FSM_IGNORE_EVENT: non evolve ma registra
			}

			static const event_tuple_t lo_stato1[] = {
				{ evn1, NULL,      FSM_NULL_STATE_ID },		quando capita evn1 non si fa nulla
				{ evn2, stt1_evn2, stt2 }					qui invece viene invocata la stt1_evn2
			} ;

		Macchina a stati

			Gli stati vanno inseriti in una tabella, assieme alle (eventuali) funzioni
			da invocare all'ingresso e all'uscita dallo stoto

			static void stt2_entra(void)
			{
				...
			}

			static const state_tuple_t fsm_tab[] = {
				{ stt1, lo_stato1, NULL, NULL },
				{ stt2, lo_stato2, stt2_entra, NULL }
				{ FSM_NULL_STATE_ID, NULL }
			} ;

		La macchina deve essere creata:

			fsm_t * fsm = NULL ;
			ASSERT(
					RC_FSM_OK ==
					fsm_create(&fsm,
							   "nome ms",
							   stt1,
							   desc_stt,
							   desc_evn,
							   fsm_tab)
				   )

		Quando capita un evento si invoca:

			ASSERT(
					RC_FSM_OK ==
					fsm_engine(fsm,
           	   	   	   	   	   evn1,
           	   	   	   	   	   NULL)
           	   	  )

*/


#ifndef __FSM_H__
#define __FSM_H__

#include "fsm_cfg.h"

/*
 * return codes utilized by the fsm APIs and
 * user event handlers
 */
typedef enum {
    /* indicates success - state change */
    RC_FSM_OK = 0,

    /* indicates that the fsm handle was NULL */
    RC_FSM_NULL,

    /* indicates that the fsm handle pointed to somethinf else! */
    RC_FSM_INVALID_HANDLE,

	// Usato nella fsm_record_history e, da me, anche come rc
    RC_FSM_INVALID_EVENT_HANDLER,

    /* indicates that an error was found in the state table */
    RC_FSM_INVALID_STATE_TABLE,

    /* indicates that the next state was out of bounds */
    RC_FSM_INVALID_STATE,

    /* indicates that an error was found in the event table */
    RC_FSM_INVALID_EVENT_TABLE,

    /* indicates that the next event was out of bounds */
    RC_FSM_INVALID_EVENT,

    /* indicates that there was no memory available */
    RC_FSM_NO_RESOURCES,

    /* event handler indicating that the event can be
     * ignored, no transition
     */
    RC_FSM_IGNORE_EVENT,

    /* event handler indicating that the state machine is
     * being deallocated and no further access to the fsm
     * strucutre should be made - history
     */
    RC_FSM_STOP_PROCESSING,
} RC_FSM_t;



#define FSM_NULL_STATE_ID   ((uint32_t) -1)
#define FSM_NULL_EVENT_ID   ((uint32_t) -1)

typedef void (*entra_esci_stato)(void) ;


/*
 * typedef RC_FSM_t (*event_cb_t)(void *p2event, void *p2parm)
 *
 * DESCRIPTION
 *    Provides typedef for the event handler functions.
 *
 *    Once the state's normalized event ID and index has been
 *    determined, the associated event call-back is invoked
 *    to effect the state processing and transition.
 *
 * INPUT PARAMETERS
 *    p2event        pointer to the raw event to be processed
 *                   by the event handler.
 *
 *    p2parm         pointer parameter that is simply
 *                   passed through from fsm_engine to each
 *                   event handler.
 *
 * RETURN VALUE
 *    RC_FSM_OK
 *    error otherwise
 */

/*
	In pratica:
		*) fsm_engine(., ., p) -> event_cb_t(p)
		*) si restituisce:
			RC_FSM_STOP_PROCESSING: non succede niente
			RC_FSM_IGNORE_EVENT	  : non succede niente ma viene registrato
			RC_FSM_OK			  : si passa allo stato predefinito
									o a quello di fsm_set_exception_state
*/

typedef RC_FSM_t (*event_cb_t)(void *p2event);


/*
 * User provided Normalized Event Description Table.
 *
 * An Example
 *    typedef enum {
 *        start_init_e = 0,
 *        init_rcvd_e,
 *        init_tmo_e,
 *        init_ack_e,
 *        start_term_e,
 *        term_rcvd_e,
 *        term_ack_e,
 *    } session_events_e;
 *
 *    static event_description_t normalized_event_table[] =
 *       {{start_init_e, "Start Session Init"},
 *        {init_rcvd_e, "Session Init"},
 *        {init_tmo_e, "Session Init ACK TMO"},
 *        {init_ack_e, "Session Init ACK"},
 *        {start_term_e, "Start Session Termination"},
 *        {term_rcvd_e, "Session Terminate"},
 *        {term_ack_e, "Session Terminate ACK"},
 *        {FSM_NULL_EVENT_ID, NULL}};     / required to end table /
 *
 */
typedef struct {
    uint32_t event_id;
    // anche NULL per evitare stampe eccessive
    const char * description ;
} event_description_t;


/*
 * User provided Normalized State DESCRIPTION Table.
 *
 * An example
 *     typedef enum {
 *         idle_s = 0,
 *         wait_for_init_ack_s,
 *         established_s,
 *         wait_for_term_ack_s,
 *     } demo_states_e;
 *
 * static state_description_t normalized_state_table[] =
 *    {{start_init_e, "Start Session Init"},
 *     {idle_s, Idle State"},
 *     {wait_for_init_ack_s, "Wait for Init Ack State"},
 *     {established_s, "Established State"},
 *     {wait_for_term_ack_s, "Wait for Terminate Ack State"},
 *     {FSM_NULL_STATE_ID, NULL}};       / required to end table /
 *
 */
typedef struct {
    uint32_t state_id;
    const char * description ;
} state_description_t;



/*
 * User provided Event Table - one table per state is
 * required. The table must have an entry for each
 * normalized  event, ordered from 0 - n.
 *
 * event_id is the normalized value.
 *
 * event_handler is the user provided call-back function that is
 *      invoked to handle the event and effect the state transition.
 *      If the event_handler is NULL, no processing is
 *      associated with the event - no state transition.
 *
 * next_state is the next state as result of the event. It is
 *          possible that a state transition associated with an event
 *          remain in the current state.
 *
 * An example:
 *   static event_tuple_t  state_wait_for_init_ack_events[] =
 *    {{start_init_e,    event_ignore,            wait_for_init_ack_s},
 *      {init_rcvd_e,     event_ignore,            wait_for_init_ack_s},
 *      {init_tmo_e,      event_init_ack_tmo,      wait_for_init_ack_s},
 *      {init_ack_e,      event_init_ack_rcvd,     established_s},
 *      {start_term_e,    event_term_rcvd,         wait_for_term_ack_s},
 *      {term_rcvd_e,     event_term_rcvd,         idle_s},
 *      {term_ack_e,      event_ignore,            wait_for_term_ack_s}};
 */
typedef struct {
    uint32_t    eventID;        /* normalized event id */
    event_cb_t  event_handler;  /* if NULL==no transaction processing */
    uint32_t    next_state;     /* normalized next state - can be
                                 * the same state */
} event_tuple_t;


/*
 * User provided Normalized State Table.  This table represents all
 * the states and the event table associated with each state.
 *
 * An example:
 *    static state_tuple_t  demo_state_table[] =
 *       {{idle_s,                        state_idle_events},
 *        {wait_for_init_ack_s,           state_wait_for_init_ack_events},
 *        {established_s,                 state_established_events},
 *        {wait_for_term_ack_s,           state_wait_for_term_ack_events},
 *        {FSM_NULL_STATE_ID, NULL}};          / requied to end table /
 */
typedef struct {
    uint32_t        state_id;
    const event_tuple_t  *p2event_tuple;
    // MZ
    entra_esci_stato pfEntra ;
    entra_esci_stato pfEsci ;
} state_tuple_t;


/*
 * Historical record of state changes
 */
typedef struct {
    uint32_t   number;
    uint32_t   prevStateID;
    uint32_t   stateID;
    uint32_t   eventID;
    RC_FSM_t handler_rc;
} fsm_history_t;


/*
 * Finite State Machine structure
 *
 */
#define FSM_TAG          ( (uint32_t) 0xBA5EBA11 )

typedef struct {
    /* for fsm validation */
    uint32_t         tag;

    uint32_t         curr_state;
    uint32_t         next_state;

    /*
     * This is set by an event handle to cover an excpetion
     * state transition that is different from the event
     * table next state transition.
     */
    uint32_t       exception_state;
    bool      exception_state_indicator;

    char           fsm_name[FSM_NAME_LEN];

    /* number states in table */
    uint32_t       number_states;

    /* number events in each state-event table */
    uint32_t       number_events;
#if 0
    /* debug and trace flags*/
    uint32_t       flags;
#endif
    /* pointer to the state table */
    const state_tuple_t  *state_table ;

    /* description of normalized states and events */
    const state_description_t  *state_description_table ;
    const event_description_t  *event_description_table ;

    /* starts at 0 and wraps */
    uint32_t       history_index;
#if FSM_HISTORY > 0
    /* memory is malloc'ed to record history */
    fsm_history_t *history;
#endif
} fsm_t;


/*
 * show state machine table
 */
extern void
fsm_display_table(fsm_t *fsm);


/*
 * shows state machine history
 */
extern void
fsm_show_history(fsm_t *fsm);


/* get state */
extern RC_FSM_t
fsm_get_state(fsm_t *fsm, uint32_t *p2state);

/*
 * utile dall'esterno: occorre che le
 * transizioni di stato siano coerenti anche
 * quando imposte
 */
extern void fsm_forza_stato(fsm_t *, uint32_t) ;


/*
 * allows event handler to update the next state based upon
 * an unexpected condition when processing the event.
 */
extern RC_FSM_t
fsm_set_exception_state(fsm_t *fsm, uint32_t exception_state);


/*
 * destroy a state machine
 */
extern RC_FSM_t
fsm_destroy(fsm_t **fsm);


/*
 * create and config a state machine
 */
extern RC_FSM_t
fsm_create(fsm_t **fsm,
           const char *fsm_name,
           uint32_t initial_state,
           const state_description_t *state_description_table,
           const event_description_t *event_description_table,
           const state_tuple_t *state_table) ;


/*
 * API to drive a state machine
 */
extern RC_FSM_t
fsm_engine(fsm_t *fsm,
           uint32_t normalized_event,
           void *p2event_buffer) ;


#endif  /* __FSM_H__ */

