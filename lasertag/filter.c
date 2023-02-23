#include "filter.h"
#include "queue.h"
#define FIR_COEF_COUNT 81
#define FREQUENCY_COUNT 10
#define IIR_COEF_COUNT 11
#define X_QUEUE_SIZE 200
#define Y_QUEUE_SIZE 200
#define Z_QUEUE_SIZE 11
#define OUTPUT_QUEUE_SIZE 2000
#define X_QUEUE_NAME "X_QUEUE"
#define Y_QUEUE_NAME "Y_QUEUE"
#define Z_QUEUE_NAME "Z_QUEUE"
#define OUTPUT_QUEUE_NAME "OUTPUT_QUEUE"


const static double fir_coeffs[FIR_COEF_COUNT] = {-4.0380596404309831e-04,-7.9577223998132733e-05, 8.7633547397507930e-05, 3.3102649123503876e-04, 6.1690783263561701e-04, 8.8405860972085699e-04, 1.0505257871433094e-03, 1.0285915467799598e-03, 7.4636589578486157e-04, 1.7222108333839658e-04,-6.6350736206008030e-04,-1.6559180288871847e-03,-2.6277586694828138e-03,-3.3499414490249868e-03,-3.5802265288666689e-03,-3.1155455416604729e-03,-1.8491574082627872e-03, 1.7929502503647278e-04, 2.7537778045449851e-03, 5.4831484749950331e-03, 7.8404300811557165e-03, 9.2399360805396225e-03, 9.1440822658465561e-03, 7.1840663297145840e-03, 3.2729970602708106e-03,-2.3120118675629530e-03,-8.8998900853130437e-03,-1.5461991989572281e-02,-2.0718391352137090e-02,-2.3305053605543580e-02,-2.1980504164829395e-02,-1.5840971253551234e-02,-4.5081634248045370e-03, 1.1744816429637631e-02, 3.1956182530486467e-02, 5.4540423466312576e-02, 7.7452669793101780e-02, 9.8432707219766644e-02, 1.1529453794642085e-01, 1.2621836829419633e-01, 1.3000000000000000e-01, 1.2621836829419633e-01, 1.1529453794642085e-01, 9.8432707219766644e-02, 7.7452669793101780e-02, 5.4540423466312576e-02, 3.1956182530486467e-02, 1.1744816429637631e-02,-4.5081634248045370e-03,-1.5840971253551234e-02,-2.1980504164829395e-02,-2.3305053605543580e-02,-2.0718391352137090e-02,-1.5461991989572281e-02,-8.8998900853130437e-03,-2.3120118675629530e-03, 3.2729970602708106e-03, 7.1840663297145840e-03, 9.1440822658465561e-03, 9.2399360805396225e-03, 7.8404300811557165e-03, 5.4831484749950331e-03, 2.7537778045449851e-03, 1.7929502503647278e-04,-1.8491574082627872e-03,-3.1155455416604729e-03,-3.5802265288666689e-03,-3.3499414490249868e-03,-2.6277586694828138e-03,-1.6559180288871847e-03,-6.6350736206008030e-04, 1.7222108333839658e-04, 7.4636589578486157e-04, 1.0285915467799598e-03, 1.0505257871433094e-03, 8.8405860972085699e-04, 6.1690783263561701e-04, 3.3102649123503876e-04, 8.7633547397507930e-05,-7.9577223998132733e-05,-4.0380596404309831e-04};
const static double iir_a_coeff[FILTER_FREQUENCY_COUNT][IIR_COEF_COUNT] = {{1.0000000000000000e+00,-5.9637727070164015e+00, 1.9125339333078248e+01,-4.0341474540744173e+01, 6.1537466875368821e+01,-7.0019717951472188e+01, 6.0298814235238872e+01,-3.8733792862566290e+01, 1.7993533279581058e+01,-5.4979061224867651e+00, 9.0332828533799547e-01
},{ 1.0000000000000000e+00,-4.6377947119071443e+00, 1.3502215749461572e+01,-2.6155952405269755e+01, 3.8589668330738348e+01,-4.3038990303252632e+01, 3.7812927599537133e+01,-2.5113598088113793e+01, 1.2703182701888094e+01,-4.2755083391143520e+00, 9.0332828533800291e-01
},{ 1.0000000000000000e+00,-3.0591317915750960e+00, 8.6417489609637634e+00,-1.4278790253808875e+01, 2.1302268283304372e+01,-2.2193853972079314e+01, 2.0873499791105537e+01,-1.3709764520609468e+01, 8.1303553577932188e+00,-2.8201643879900726e+00, 9.0332828533800769e-01
},{ 1.0000000000000000e+00,-1.4071749185996747e+00, 5.6904141470697471e+00,-5.7374718273676217e+00, 1.1958028362868873e+01,-8.5435280598354382e+00, 1.1717345583835918e+01,-5.5088290876998407e+00, 5.3536787286077372e+00,-1.2972519209655518e+00, 9.0332828533799414e-01
},{ 1.0000000000000000e+00, 8.2010906117760141e-01, 5.1673756579268559e+00, 3.2580350909220819e+00, 1.0392903763919172e+01, 4.8101776408668879e+00, 1.0183724507092480e+01, 3.1282000712126603e+00, 4.8615933365571822e+00, 7.5604535083144497e-01, 9.0332828533799658e-01
},{ 1.0000000000000000e+00, 2.7080869856154512e+00, 7.8319071217995688e+00, 1.2201607990980744e+01, 1.8651500443681620e+01, 1.8758157568004549e+01, 1.8276088095999022e+01, 1.1715361303018897e+01, 7.3684394621253499e+00, 2.4965418284511904e+00, 9.0332828533800436e-01
},{ 1.0000000000000000e+00, 4.9479835250075892e+00, 1.4691607003177602e+01, 2.9082414772101060e+01, 4.3179839108869331e+01, 4.8440791644688879e+01, 4.2310703962394342e+01, 2.7923434247706432e+01, 1.3822186510471010e+01, 4.5614664160654357e+00, 9.0332828533799958e-01
},{ 1.0000000000000000e+00, 6.1701893352279846e+00, 2.0127225876810336e+01, 4.2974193398071684e+01, 6.5958045321253451e+01, 7.5230437667866596e+01, 6.4630411355739852e+01, 4.1261591079244127e+01, 1.8936128791950534e+01, 5.6881982915180291e+00, 9.0332828533799803e-01
},{ 1.0000000000000000e+00, 7.4092912870072398e+00, 2.6857944460290135e+01, 6.1578787811202247e+01, 9.8258255839887312e+01, 1.1359460153696298e+02, 9.6280452143026082e+01, 5.9124742025776392e+01, 2.5268527576524203e+01, 6.8305064480743081e+00, 9.0332828533799969e-01
},{ 1.0000000000000000e+00, 8.5743055776347692e+00, 3.4306584753117889e+01, 8.4035290411037053e+01, 1.3928510844056814e+02, 1.6305115418161620e+02, 1.3648147221895786e+02, 8.0686288623299745e+01, 3.2276361903872115e+01, 7.9045143816244696e+00, 9.0332828533799636e-01}};

const static double iir_b_coeff[FILTER_FREQUENCY_COUNT][IIR_COEF_COUNT] = {{ 9.0928661148194738e-10, 0.0000000000000000e+00,-4.5464330574097372e-09, 0.0000000000000000e+00, 9.0928661148194745e-09, 0.0000000000000000e+00,-9.0928661148194745e-09, 0.0000000000000000e+00, 4.5464330574097372e-09, 0.0000000000000000e+00,-9.0928661148194738e-10
},{ 9.0928661148185608e-10, 0.0000000000000000e+00,-4.5464330574092806e-09, 0.0000000000000000e+00, 9.0928661148185613e-09, 0.0000000000000000e+00,-9.0928661148185613e-09, 0.0000000000000000e+00, 4.5464330574092806e-09, 0.0000000000000000e+00,-9.0928661148185608e-10
},{ 9.0928661148182951e-10, 0.0000000000000000e+00,-4.5464330574091475e-09, 0.0000000000000000e+00, 9.0928661148182949e-09, 0.0000000000000000e+00,-9.0928661148182949e-09, 0.0000000000000000e+00, 4.5464330574091475e-09, 0.0000000000000000e+00,-9.0928661148182951e-10
},{ 9.0928661148210734e-10, 0.0000000000000000e+00,-4.5464330574105371e-09, 0.0000000000000000e+00, 9.0928661148210742e-09, 0.0000000000000000e+00,-9.0928661148210742e-09, 0.0000000000000000e+00, 4.5464330574105371e-09, 0.0000000000000000e+00,-9.0928661148210734e-10
},{ 9.0928661148197561e-10, 0.0000000000000000e+00,-4.5464330574098779e-09, 0.0000000000000000e+00, 9.0928661148197557e-09, 0.0000000000000000e+00,-9.0928661148197557e-09, 0.0000000000000000e+00, 4.5464330574098779e-09, 0.0000000000000000e+00,-9.0928661148197561e-10
},{ 9.0928661148179839e-10, 0.0000000000000000e+00,-4.5464330574089919e-09, 0.0000000000000000e+00, 9.0928661148179839e-09, 0.0000000000000000e+00,-9.0928661148179839e-09, 0.0000000000000000e+00, 4.5464330574089919e-09, 0.0000000000000000e+00,-9.0928661148179839e-10
},{ 9.0928661148193684e-10, 0.0000000000000000e+00,-4.5464330574096843e-09, 0.0000000000000000e+00, 9.0928661148193686e-09, 0.0000000000000000e+00,-9.0928661148193686e-09, 0.0000000000000000e+00, 4.5464330574096843e-09, 0.0000000000000000e+00,-9.0928661148193684e-10
},{ 9.0928661148195069e-10, 0.0000000000000000e+00,-4.5464330574097538e-09, 0.0000000000000000e+00, 9.0928661148195076e-09, 0.0000000000000000e+00,-9.0928661148195076e-09, 0.0000000000000000e+00, 4.5464330574097538e-09, 0.0000000000000000e+00,-9.0928661148195069e-10
},{ 9.0928661148190954e-10, 0.0000000000000000e+00,-4.5464330574095478e-09, 0.0000000000000000e+00, 9.0928661148190956e-09, 0.0000000000000000e+00,-9.0928661148190956e-09, 0.0000000000000000e+00, 4.5464330574095478e-09, 0.0000000000000000e+00,-9.0928661148190954e-10
},{ 9.0928661148206091e-10, 0.0000000000000000e+00,-4.5464330574103047e-09, 0.0000000000000000e+00, 9.0928661148206094e-09, 0.0000000000000000e+00,-9.0928661148206094e-09, 0.0000000000000000e+00, 4.5464330574103047e-09, 0.0000000000000000e+00,-9.0928661148206091e-10}};

static double current_power[FILTER_FREQUENCY_COUNT];
static queue_t xQueue;
static queue_t yQueue;
static queue_t zQueues[FILTER_FREQUENCY_COUNT];
static queue_t outputQueues[FILTER_FREQUENCY_COUNT];


static void init_xQueue(){
    queue_init(&xQueue, FIR_COEF_COUNT, X_QUEUE_NAME);
    for(int i = 0; i < xQueue.size; i++)
    {
        queue_overwritePush(&xQueue, 0);
    }
}
static void init_yQueue(){
    queue_init(&yQueue, Y_QUEUE_SIZE, Y_QUEUE_NAME);
    for(int i = 0; i < yQueue.size; i++)
    {
        queue_overwritePush(&yQueue, 0);
    }
}
static void init_zQueue(){
    for(int n = 0; n < FILTER_FREQUENCY_COUNT; n++)
    {
        queue_init(&zQueues[n], Z_QUEUE_SIZE, Z_QUEUE_NAME);
        for(int i = 0; i < zQueues[n].size; i++)
        {
            queue_overwritePush(&zQueues[n], 0);
        }
    }
}

static void init_outputQueues(){
    for(int n = 0; n < FILTER_FREQUENCY_COUNT; n++)
    {
        queue_init(&outputQueues[n], OUTPUT_QUEUE_SIZE, OUTPUT_QUEUE_NAME);
        for(int i = 0; i < outputQueues[n].size; i++)
        {
            queue_overwritePush(&outputQueues[n], 0);
        }
    }
}

// Must call this prior to using any filter functions.
void filter_init(){
    printf("\n");
    init_xQueue();
    init_yQueue();
    init_zQueue();
    init_outputQueues();
}

// Use this to copy an input into the input queue of the FIR-filter (xQueue).
void filter_addNewInput(double x){
    queue_overwritePush(&xQueue, x);
}

// Invokes the FIR-filter. Input is contents of xQueue.
// Output is returned and is also pushed on to yQueue.
double filter_firFilter(){
    double y = 0;
    for(int i = 0; i < FIR_COEF_COUNT; i++)
    {
        y += fir_coeffs[i] * queue_readElementAt(&xQueue, FIR_COEF_COUNT - 1 - i);
    }
    queue_overwritePush(&yQueue, y);
    return y;
}

// Use this to invoke a single iir filter. Input comes from yQueue.
// Output is returned and is also pushed onto zQueue[filterNumber].
double filter_iirFilter(uint16_t filterNumber){
    double z = 0;
    
    z += iir_b_coeff[filterNumber][0]* queue_readElementAt(&yQueue, Y_QUEUE_SIZE-1);
    //z += iir_a_coeff[filterNumber][0] * queue_readElementAt(&zQueues[filterNumber], IIR_COEF_COUNT);
    //printf("extra val: %f\n", iir_b_coeff[filterNumber][0] * queue_readElementAt(&yQueue, 0));
        //printf("Filtering with b and index %d: %24.20le and %24.20le\n", 0, iir_b_coeff[filterNumber][0], queue_readElementAt(&yQueue, 0));
    for(int i = 1; i < IIR_COEF_COUNT; i++)
    {
        z += iir_b_coeff[filterNumber][i]* queue_readElementAt(&yQueue, Y_QUEUE_SIZE - 1 -i);
        //printf("da b coef: %f da q element: %f\n", iir_b_coeff[filterNumber][i], queue_readElementAt(&yQueue, i));
        //z += iir_a_coeff[filterNumber][i] * queue_readElementAt(&zQueues[filterNumber], IIR_COEF_COUNT - i);
        //printf("Filtering with b and index %d: %24.20le and %24.20le\n", IIR_COEF_COUNT - i, iir_b_coeff[filterNumber][i], queue_readElementAt(&yQueue, i));
      
        z -= iir_a_coeff[filterNumber][i] * queue_readElementAt(&zQueues[filterNumber], IIR_COEF_COUNT - i);
    }
    queue_overwritePush(&zQueues[filterNumber], z);
    queue_overwritePush(&outputQueues[filterNumber], z);
    return z;
}

// Use this to compute the power for values contained in an outputQueue.
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch,
                           bool debugPrint){
    static double oldest_value[FILTER_FREQUENCY_COUNT];
    // Sum up all the power from scratch if its the first time
    if(forceComputeFromScratch){
        double new_power = 0.0;
        for(int i = 0; i < OUTPUT_QUEUE_SIZE; i++){
            double energy = queue_readElementAt(&outputQueues[filterNumber], i);
            new_power += energy * energy;
        }
        current_power[filterNumber] = new_power;
        //printf("Initial Power: %f\n", new_power);
        oldest_value[filterNumber] = queue_readElementAt(&outputQueues[filterNumber], 0);
        return new_power;
    }

    // Otherwise, just subtract the oldest power and add the newest one
    double newest_value = queue_readElementAt(&outputQueues[filterNumber], OUTPUT_QUEUE_SIZE-1);
    double new_power = current_power[filterNumber] - oldest_value[filterNumber] * oldest_value[filterNumber] + newest_value*newest_value;
    oldest_value[filterNumber] = queue_readElementAt(&outputQueues[filterNumber], 0);
    double power_test = filter_computePower(filterNumber, true, false);
    //printf("test power: scratch: %lf update: %lf\nCurrent power: %lf\nOld power: %lf\nNew power: %lf\n", power_test, new_power, current_power[filterNumber], oldest_value[filterNumber] * oldest_value[filterNumber], newest_value*newest_value);
    current_power[filterNumber] = new_power;
    
    return new_power; 
}

// Returns the last-computed output power value for the IIR filter
// [filterNumber].
double filter_getCurrentPowerValue(uint16_t filterNumber){
    return current_power[filterNumber];
}

// Sets a current power value for a specific filter number.
// Useful in testing the detector.
void filter_setCurrentPowerValue(uint16_t filterNumber, double value){
    current_power[filterNumber] = value; 
}

// Get a copy of the current power values.
// This function copies the already computed values into a previously-declared
// array so that they can be accessed from outside the filter software by the
// detector. Remember that when you pass an array into a C function, changes to
// the array within that function are reflected in the returned array.
void filter_getCurrentPowerValues(double powerValues[]){
    for(int i = 0; i < FILTER_FREQUENCY_COUNT; i++)
    {
        powerValues[i] = current_power[i];
    }
}

// Using the previously-computed power values that are currently stored in
// currentPowerValue[] array, copy these values into the normalizedArray[]
// argument and then normalize them by dividing all of the values in
// normalizedArray by the maximum power value contained in currentPowerValue[].
// The pointer argument indexOfMaxValue is used to return the index of the
// maximum value. If the maximum power is zero, make sure to not divide by zero
// and that *indexOfMaxValue is initialized to a sane value (like zero).
void filter_getNormalizedPowerValues(double normalizedArray[],
                                     uint16_t *indexOfMaxValue){
    *indexOfMaxValue = 0;
    // Find the index of the max power
    for(int i = 1; i < FILTER_FREQUENCY_COUNT; i++){
        if(current_power[*indexOfMaxValue] < current_power[i]){
            *indexOfMaxValue = i;
        }
    }
    // Dont divide by 0
    if(current_power[*indexOfMaxValue] == 0){
        return;
    }
    // Normalize the power array
    for(int i = 0; i < FILTER_FREQUENCY_COUNT; i++){
        normalizedArray[i] = current_power[i]/current_power[*indexOfMaxValue];
    }
}

/******************************************************************************
***** Verification-Assisting Functions
***** External test functions access the internal data structures of filter.c
***** via these functions. They are not used by the main filter functions.
******************************************************************************/

// Returns the array of FIR coefficients.
const double *filter_getFirCoefficientArray(){
    return fir_coeffs;
}

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount(){
    return FIR_COEF_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber){
    return iir_a_coeff[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount(){
    return IIR_COEF_COUNT;
}

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber){
    return iir_b_coeff[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount(){
    return IIR_COEF_COUNT;
}

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize(){
    return yQueue.size;
}

// Returns the decimation value.
uint16_t filter_getDecimationValue(){
    return FILTER_FIR_DECIMATION_FACTOR;
}

// Returns the address of xQueue.
queue_t *filter_getXQueue(){
    return &xQueue;
}

// Returns the address of yQueue.
queue_t *filter_getYQueue(){
    return &yQueue;
}

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber){
    return zQueues + filterNumber;
}

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber){
    return outputQueues + filterNumber;
}