#ifndef __PATIENT_H__
#define __PATIENT_H__

/*
 * This header file is associated with the clinics patient records.  Each
 * event listed in the pt_event_e (patient event) below should be recorded
 * as they take place.  Refer to each event for further details explaining
 * when to record the event and what data should be recorded.
 */

/*
 * Enumeration which defines the patient event taking place.  Refer to
 * the comments to see when the event should be delivered.  Some events
 * require additional information to be stored in a corresponding
 * structure.  Refer to the structure for additional information.
 */
typedef enum {
	PTEVT_NONE = 0,
	PTEVT_CLINIC_ARRIVAL,        /* ptevt_register_t: patient arrives at the clinic (includes returning)  */
	PTEVT_CLINIC_DEPARTURE,      /* ptevt_register_t: patient leaves the clinic (including temporarily) */
	PTEVT_WAITROOM_ARRIVAL,      /* None:             patient sits in waiting room */
	PTEVT_WAITROOM_DEPARTURE,    /* None:             patient leaves waiting room */
	PTEVT_DOCTOR_TREATSTART,     /* ptevt_dr_t:       doctor starts or resumes treating a patient */
	PTEVT_DOCTOR_TREATSTOP,      /* ptevt_dr_t:       doctor is finished treating a patient */
	PTEVT_SPECIALIST_TREATSTART, /* ptevt_dr_t:       specialist starts treating a patient */
	PTEVT_SPECIALIST_TREATSTOP,  /* ptevt_dr_t:       specialist is finished treating a patient */
        PTEVT_APT_INTRREQUEST,       /* ptevt_intr_t:     a VIP patient tried to interrupt a patient's appointment */
	PTEVT_WAITLIST_ADDED,        /* None:             patient is added to the callback waitlist */
	PTEVT_WAITLIST_REMOVED,      /* None:             patient is removed from the callback waitlist (they were called) */
	PTEVT_FEEBACK,               /* ptevt_feedback_t  feedback was provided by a patient at the end of an appointment */
	PTEVT_COUNT,                 /* Sentinel value, not used */
} pt_event_e;

/*
 * Structure used when a patient arrives or departs from the clinic.  A
 * random ailment based on ailment_e in clinic.h should be provided unless
 * the patient is returning after being interrupted by a VIP patient, or
 * after getting a callback while in the coffee shop. If the patient is
 * returning, the ailment should be set to the ailment they originally specified.
 * The apt_length should reflect how much time is left in the appointment.  If
 * the patient is not returning, the apt_length should be set to the corresponding
 * time in the comments of ailment_e in clinic.h.  The membership field should
 * denote their current membership priority from membership_e in clinic.h.
 *
 * Note: Refer to ailment_e in clinic.h for random ailments and associated
 *       appointment times.
 *
 * This structure is used with  entries:
 *
 *   PTEVT_CLINIC_ARRIVAL
 *   PTEVT_CLINIC_DEPARTURE
 *
 * Fields:
 *  -- ailment:    Random ailment to be treated for.
 *  -- membership  Patient's current membership level.
 *  -- apt_length: Length remaining in the upcoming appointment.
 */
typedef struct ptevt_register_s {
	ailment_e      ailment;
	membership_e   membership;
	uint32_t       apt_length;
} ptevt_register_t;

/*
 * Structure used when a patient begins or concludes an appointment with a
 * doctor or a specialist.  The time_remaining field is used to record the
 * time remaining in the appointment which should match the ptevt_register_t
 * structures apt_length field.  When using the event for the end of a treatment,
 * the time_remaining should be set to 0.  The doctor_id should be set to
 * the ID of the doctor treating the patient.  If being used for a specialist,
 * use SPECIALIST_ID as the doctor_id.  This structure is used with the entries:
 *
 *   PTEVT_DOCTOR_TREATSTART
 *   PTEVT_DOCTOR_TREATSTOP
 *   PTEVT_SPECIALIST_TREATSTART
 *   PTEVT_SPECIALIST_TREATSTOP
 *
 * Fields:
 *  -- doctor_id:      ID of doctor treating patient.
 *  -- time_remaining: Amount of time left in the appointment.
 */
typedef struct ptevt_dr_s {
	uint32_t       doctor_id;
	uint32_t       time_remaining;
} ptevt_dr_t;

/* 
 * Structure used to record when a patient who is not VIP gets interrupted
 * during their appointment by a VIP member.  The time remaining in their
 * appointment should be recorded the the boolean field should record whether
 * or not the interruption was successful (remember that a specialist can't
 * be interrupted).  The interrupter_id should be set to the patient ID of
 * the VIP patient trying to interrupt the appointment.  The membership should
 * reflect their current membership or their boosted membership if they are
 * interrupted by a VIP patient.
 *
 *  PTEVT_APT_INTRREQUEST
 *
 * Fields:
 *  -- interrupter_id: Patient ID of the VIP interrupter.
 *  -- time_remaining: Amount of time left in the appointment when patient is interrupted.
 *  -- membership:     Current membership reflecting a boost if they're interrupted.
 *  -- interrupted:    Status of interrupt attempt (true or false).
 */
typedef struct ptevt_intr_s {
	uint32_t       interrupter_id;
	uint32_t       time_remaining;
	membership_e   membership;
	bool           interrupted;
} ptevt_intr_t;

/*
 * Structure used to record the number of stars a patient gives the doctor.
 * A random value between 1-10 should be provided for the rating. If a patient
 * ends up seeing a specialist, there should be two feedback events; one for
 * the doctor and one for the specialist.  Note that a doctor is still
 * considered to be treating a patient if they are conferencing with a specialist.
 * This means that these events should be back-to-back.  The events should be in
 * order or treatment so the doctor feedback event should always proceed the
 * specialist feedback event.  Note that SPECIALIST_ID should be used for
 * doctor_id when referring to the specialist.
 *
 *   PTEVT_FEEBACK_PROVIDED,
 * 
 * Fields:
 *   -- doctor_id:        ID of the doctor who treated the patient.
 *   -- doctor_stars:     stars between 1-10 the patient rated the doctor.
 */
typedef struct ptevt_feedback_s {
	uint32_t       doctor_id;
	uint32_t       doctor_rating;
} ptevt_feedback_t;

/*
 * Common patient event structure used to store different types of events.  All
 * events share a timestamp and type while some events required additional
 * information stored as part of the anonymous union.
 *
 * Fields:
 *   -- timestamp: Timestamp recording when the event occurred
 *   -- type:      Type of event which
 *   -- {union}:   Extra data required for certain events
 */
typedef struct patient_event_s {
	struct timespec    timestamp;
	pt_event_e         type;
	union {
		ptevt_register_t       reg;
		ptevt_dr_t             dr;
		ptevt_intr_t           intr;
		ptevt_feedback_t       feedback;
	};
} patient_event_t;

#endif
