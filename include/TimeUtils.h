/*==============================================================================
 * @Company
 * @Copyright
 * UNPUBLISHED WORK
 * ALL RIGHTS RESERVED
 *
 * This software is the confidential and proprietary information of
 *
 *
 *
 *
 *
 *============================================================================*/

#ifndef INCLUDE_TIMEUTILS_H_
#define INCLUDE_TIMEUTILS_H_

/*****************************************************************************
 * INCLUDE FILES
 *****************************************************************************/
/* Standard */
#include <stdint.h>
#include <sys/time.h>

/**============================================================================
 * Class description
 *
 *
 *============================================================================*/
namespace TimeUtils {


/**=========================================================================
 * @brief getDifference
 *
 * @param[in]  t1
 * @param[in]  t2
 *
 * @return     int
 *=========================================================================*/
inline int getDifference( struct timeval const *t1, struct timeval const* t2 ) {

   int ms = ( t1->tv_sec - t2->tv_sec ) * 1000;
   ms += ( t1->tv_usec - t2->tv_usec ) / 1000;
   return ms;
}

/**=========================================================================
 * @brief getDifference
 *
 *
 * @param[in]  t1
 * @param[in]  t2
 *
 * @return     int
 *=========================================================================*/
inline int getDifference( struct timespec const *t1,
      struct timespec const *t2 ) {
   int ms = ( t1->tv_sec - t2->tv_sec ) * 1000;
   ms += ( t1->tv_nsec - t2->tv_nsec ) / 1000000;
   return ms;
}

/**=========================================================================
 * @brief getDifferenceMicroSecs
 *
 *
 * @param[in]     t1
 * @param[in]     t2
 *
 * @return        int
 *=========================================================================*/
inline int getDifferenceMicroSecs( struct timeval *t1, struct timeval *t2 ) {
   int us = ( t1->tv_sec - t2->tv_sec ) * 1000000;
   us += ( t1->tv_usec - t2->tv_usec );
   return us;
}

/**=========================================================================
 * @brief getDifferenceMicroSecs
 *
 *
 * @param[in]     t1
 * @param[in]     t2
 *
 * @return        int
 *=========================================================================*/
inline int getDifferenceMicroSecs( struct timespec *t1, struct timespec *t2 ) {
   int us = ( t1->tv_sec - t2->tv_sec ) * 1000000;
   us += ( t1->tv_nsec - t2->tv_nsec ) / 1000;
   return us;
}

/**=========================================================================
 * @brief getCurTime
 *
 *
 * @param[out]     t
 *
 * @return         void
 *=========================================================================*/
inline void getCurTime( struct timeval *t ) {
   gettimeofday( t, NULL );
}

/**=========================================================================
 * @brief getCurTime
 *
 *
 * @param[out]     t
 *
 * @return         void
 *=========================================================================*/
inline void getCurTime( struct timespec *t ) {
#ifdef HAS_CLOCK_GETTIME
   clock_gettime(CLOCK_REALTIME, t);
#else
   struct timeval tv;
   gettimeofday( &tv, NULL );
   t->tv_sec = tv.tv_sec;
   t->tv_nsec = tv.tv_usec * 1000;
#endif
}

/**=========================================================================
 * @brief getCurrTimeMillis
 *
 *
 * @return         uint64_t
 *=========================================================================*/
inline uint64_t getCurrTimeMillis() {
   struct timeval t;
   gettimeofday( &t, NULL );
   return (uint64_t) t.tv_sec * 1000 + (uint64_t) t.tv_usec / 1000;
}

/**=========================================================================
 * @brief getCurrTimeSecs
 *
 *
 * @return         uint64_t
 *=========================================================================*/
inline uint64_t getCurrTimeSecs() {
   struct timeval t;
   gettimeofday( &t, NULL );
   return (uint64_t) ( t.tv_sec );
}

/**=========================================================================
 * @brief setTimeStruct
 *
 * @param[in]     msecs
 * @param[out]    *t
 * @return        void
 *=========================================================================*/
inline void setTimeStruct( struct timespec *t, uint32_t msecs ) {
   t->tv_sec = msecs / 1000;
   t->tv_nsec = ( msecs % 1000 ) * 1000000;
}

/**=========================================================================
 * @brief setTimeStruct
 *
 * @param[in]     msecs
 * @param[out]    *t
 * @return        void
 *=========================================================================*/
inline void setTimeStruct( struct timeval *t, uint32_t msecs ) {
   t->tv_sec = msecs / 1000;
   t->tv_usec = ( msecs % 1000 ) * 1000;
}

/**=========================================================================
 * @brief addOffset
 *
 * @param[in]     msecs
 * @param[out]    *t
 * @return        void
 *=========================================================================*/
inline void addOffset( struct timespec *t, int msecs ) {
   t->tv_sec += msecs / 1000;
   t->tv_nsec += ( msecs % 1000 ) * 1000000;

   if ( t->tv_nsec >= 1000000000 ) {
      t->tv_sec++;
      t->tv_nsec -= 1000000000;
   } else if ( t->tv_nsec < 0 ) {
      t->tv_sec--;
      t->tv_nsec += 1000000000;
   }
}

/**=========================================================================
 * @brief addOffset
 *
 * @param[in]     msecs
 * @param[out]    *t
 * @return        void
 *=========================================================================*/
inline void addOffset( struct timeval *t, int msecs ) {
   t->tv_sec += msecs / 1000;
   t->tv_usec += ( msecs % 1000 ) * 1000;

   if ( t->tv_usec >= 1000000 ) {
      t->tv_sec++;
      t->tv_usec -= 1000000;
   } else if ( t->tv_usec < 0 ) {
      t->tv_sec--;
      t->tv_usec += 1000000;
   }
}

};

#endif /* ifndef INCLUDE_TIMEUTILS_H_ */
