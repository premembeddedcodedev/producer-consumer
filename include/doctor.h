#ifndef __DOCTOR_H__
#define __DOCTOR_H__

/*
 * This header file is associated with the clinics doctor records.  Each event
 * listed in the dr_event_e (doctor event) below should be recorded as they
 * take place.  Refer to each event for further details explaining when to
 * record the event and what data should be recorded.
 */

/*
 * Enumeration which defines the doctor event taking place.  Refer to
 * the comments to see when the event should be delivered.  Some events
 * require additional information to be stored in a corresponding
 * structure.  Refer to the structure in the corresponding comment
 * for additional information.
 */
typedef enum {
	DREVT_NONE = 0,
	DREVT_SHIFTSTART,        /* None:             Doctor's day is starting */
	DREVT_SHIFTEND,          /* None:             Doctor's day is ending */
	DREVT_WAITING,           /* drevt_status_t:   Doctor is idle as no patients are available to treat */
	DREVT_WOKENUP,           /* drevt_status_t:   Doctor is woken up by a patient who needs treating */
	DREVT_TREATSTART,        /* drevt_aptmt_t:    Doctor is starting an appointment */
	DREVT_TREATEND,          /* drevt aptmt_t:    Doctor is ending the appointment */
        DREVT_SPECIALSTART,      /* drevt_aptmt_t:    Doctor is calling the specialist */
        DREVT_SPECIALEND,        /* drevt_aptmt_t:    Doctor is done consulting with the specialist */
	DREVT_COUNT,             /* Sentinel value, not used */
} dr_event_e;

/*
 * Structure used when a doctor either begins waiting for a patient to
 * wake them up or when a patient has just woken them up.  If the event
 * is DREVT_WAITING, the patient_id should be set to 0, otherwise the
 * patient_id should be set to the patient ID who woke us up.  The
 * idle_count is the number of times the doctor has been woken up during
 * their shift and the idle_time is the the total amount of time the
 * doctor has been without a patient.  patients_treated represented the
 * total number of patients the doctor has treated so far.
 *
 *   DREVT_WAITING
 *   DREVT_WOKENUP
 *
 * Fields:
 *  -- patient_id:       ID of the patient that woke us up or 0 if waiting.
 *  -- patients_treated: Total number of patients we've treated so far.
 *  -- idle_count:       Total number of times we've been in an idle state.
 *  -- idle_time:        Total time we've been idle so far.
 */
typedef struct drevt_status_s {
	uint32_t        patient_id;
	uint32_t        patients_treated;
	uint32_t        idle_count;
	uint32_t        idle_time;
} drevt_status_t;

/*
 * Structure used when a doctor is treating/finished treating a patient
 * and when a doctor needs to teleconference with a specialist.  When
 * no specialist is required, the specialist_id should be set to 0,
 * otherwise, SPECIALIST_ID should be used.  When no specialist is
 * required, the apt_length is the appointment length the patient
 * was either provided at check-in or the time remaining if their
 * previous appointment was interrupted.  If a specialist is being
 * called in, the apt length should be 5 as all specialist appointments
 * take 7 units of time.
 *
 *   DREVT_TREATSTART
 *   DREVT_TREATEND
 *
 * Fields:
 *  -- patient_id:    ID of the patient we're treating.
 *  -- specialist_id: ID of the specialist we're consulting with.
 *  -- apt_length:    Projected length of the appointment.
 *  -- ailment:       Ailment of the patient we're treating.
 */
typedef struct drevt_aptmt_s {
	uint32_t       patient_id;
	uint32_t       specialist_id;
	uint32_t       apt_length;
	ailment_e      ailment;
} drevt_aptmt_t;

/*
 * Structure used to store events of different types.  Each event shares
 * a type and and timestamp.  Some events required additional information
 * and are stored in their respective types.
 *
 * Fields:
 *   -- timestamp: Timestamp when the event occurred
 *   -- type:      Type of event which determines the underlying structure if any
 */
typedef struct doctor_event_s {
	struct timespec    timestamp;
	dr_event_e         type;
	union {
		drevt_status_t    status;
		drevt_aptmt_t     aptmt;
	};
} doctor_event_t;

#endif
