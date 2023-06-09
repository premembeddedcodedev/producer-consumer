# producer-consumer

Usage of this package:
1. clone the package
2. build the application using make
3. run the application using ./test
4. change the inputs in client.h in include folder
5. it will run all the scenarios automatially

Algorithm:

there are 2 Queues processed threads created as follows:
 1. WaitQueue
 2. Callback queue
 
Waitqueue continuously generate the traffic and ensure sending patients in asending order to doctors pool and when queue is full, it will enqueue the patients to callback queue.

created doctors threads, scenarious covered are
1. As normal subscription patient enters into doctors thread and wait for the signal from waitqueue thread - 
   a. timout - safely process and wait for another patients from waitqueue thread
   b. signal - when signal arrives from waitqueue, it will detect and enqueue it into WQ incase its free otherwise it enques on to callback queue front and fill the doctors details and wait for patients from wait queue thread.

2. incase doctor got subscription as vip it will wait for designated amount of time

3. doctor thread will generate the vip consiltation randomly for each 4 members of patients it will process and never terminates till doctor thread relinquish the threas time

4. critical section has been kept for dequeing and data

another thread is callback queue thread which has been notified when WQ has no member to serve.

Scenarios:

A new privatized health clinic just opened.  They have a 4 tier pricing model increasing in priority; Silver: 1, Gold: 2, Platinum: 3, and VIP: 4.  This clinic has D doctors, C chairs available in the waiting room, and 1 remote specialist.  The level of care between the pricing models is equivalent, but provides priority status for higher tiers.  When a patient arrives, they must take a seat in the waiting room before seeing a doctor.  If there is no space in the waiting room, the patient heads to the coffee shop next door and waits for a call letting them know there is a spot available in the waiting room.  Doctors are in a hurry to get home for the day and try and treat patients as fast as they can.  They always treat patients by ascending priority.  Sometimes the doctor will need to perform a teleconference with a specialist which will take a set amount of time.  This teleconference will take place immediately following the regular appointment.
 
The priority model has some rules.  If the waiting room is full, a lower priority patient is kicked out by  a higher priority patient and heads to the coffee shop to await a callback.  Callbacks are also done on a priority basis.  If a lower priority patient is seeing a doctor, they can finish their visit if a higher priority patient enters the waiting room with one exception; VIP patients pay such an exorbitant amount that they can interrupt the doctor’s appointment of a lower priority patient.  There is one caveat to to this; the specialist has a bit of an ego and refuses to be interrupted.  This means that if a VIP patient tries to interrupt a lower priority patient’s appointment and they’re on a remote call with the specialist, tihey must wait for the specialist to finish.  If the VIP patient must wait, normal waiting room and callback rules apply.  The clinic isn’t completely heartless and will temporarily bump any interrupted patient to ‘Platinum’ status for the remainder of their visit so they can get back in to see their doctor as quickly as possible.  This happens the instant a patient is interrupted.  It’s important that the interrupted patient returns to the same doctor they were visiting before the interruption and that their original membership is restored immediately at the end of the doctor visit.
 
This clinic takes impeccable records.  Everything is recorded throughout the day.  At the end of the day, all doctor and patient records are printed out sequentially; first doctor records, then patient records.  Doctors record all the data themselves and their records are never shared with anyone else.  Refer to doctors.h for a complete list of what should be recorded, and when.  Note that specialists follow their own rules and we aren’t concerned with tracking their records.  Patient records differ slightly from doctor records as they can be access by doctors, patients, and reception at any time.   Take this into account when designing your data structures.  Refer to the comments and structures in patients.h for a complete list of what should be recorded, and when.
 
In this scenario,  D doctors arrive first and are given a sequential doctor ID.  The D doctors have P patients they need to treat for the day.  Patients will arrive sequentially at the clinic between 4 and 10 units of time from the time of the last patient.  Once they arrive, they check in with reception and are given a sequential patient ID and provide an ailment they want cured which you should randomly select.  Refer to ailment_e in patient.h.  This clinic is very strict and each ailment takes an exact amount of time to treat which is defined in the corresponding ailments comments.  Note that each patient also has a 1 in 4 chance of having to see a specialist which is determined randomly by the doctor at the end of the appointment.  If it’s determined that a patient needs to see a specialist, this takes an extra 7 units of time on top of the regularly scheduled appointment.  Note that the doctor remains in the room during this teleconference with the patient and specialist.  Remember that there are C chairs available in the waiting room and infinite room on the callback list.  At the end of the day after all patients have been treated and all doctors have left for the day, all records are printed out.
 
All D doctors and all P patients should be represented by separate threads.  Doctor and patient data should be stored in separate structures called patient_t, and doctor_t and should be stored until after they are printed at the end of the day.   The design and implementation of these structures is entirely up to you.  Be sure to read the comments in clinic.h, patient.h and doctor.h for additional information.
 
Create the multithreaded application with appropriate synchronization that depicts the Doctor’s office described above.



