#ifndef __CLINIC_H__
#define __CLINIC_H__

/*
 * There is only one specialist so this value can be used when filling
 * out any fields requiring a specialist_id.
 *
 * NOTE: a specialist always takes 7 units of time and a specialist is
 *       only called at the end of a regularly scheduled doctor's
 *       appointment.
 */
#define SPECIALIST_ID	(uint32_t)-1

/*
 * Enumeration of ailments this clinic will treat.  A random ailment
 * should be selected when a patient first registers at reception.
 * The total time this ailment will take to treat (not including any
 * time required with a specialist) is listed in the comments below.
 */
typedef enum {
	AIL_NONE = 0,              /* Default, shouldn't bet set */
	AIL_CHECKUP,               /* Takes 12 units of time */
	AIL_GENERAL_MALAISE,       /* Takes 10 units of time */
	AIL_FEVER,                 /* Takes 8 units of time */
	AIL_PHYSICAL_INJURY,       /* Takes 6 units of time */
	AIL_OTHER,                 /* Takes 4 units of time */
	AIL_COUNT                  /* Sentinel, ignore */
} ailment_e;

/*
 * Enumeration of membership tiers this clinic provides to patients.
 */
typedef enum {
	MEMBERSHIP_NONE = 0,        /* Default, shouldn't be set */
	MEMBERSHIP_SILVER,          /* Silver membership has a prioroity of 1 */
	MEMBERSHIP_GOLD,            /* Gold membership has a priority of 2 */
	MEMBERSHIP_PLATINUM,        /* Platinum membership has a priority of 3 */
	MEMBERSHIP_VIP,             /* VIP membership has a priority of 4 */
} membership_e;


#endif
