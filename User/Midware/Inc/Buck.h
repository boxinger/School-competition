#ifndef __BUCK_H
#define __BUCK_H

#include "PID.h"

typedef struct {
    void (*InitFunc)(void);
    void (*DeInitFunc)(void);
    void (*Start)(void);
    void (*Stop)(void);
    void (*SetDuty)(float duty);
    float (*GetInputVoltage)(void);
    float (*GetOutputVoltage)(void);
    float (*GetInductorCurrent)(void);
    PID_InitTypeDef* OutPutVoltagePIDInit;
    PID_InitTypeDef* InductorCurrentPIDInit;
} Buck_InitTypeDef;

typedef enum{
    BUCK_ERROR = 0U,
    BUCK_DISABLED,
    BUCK_CVMODE,
    BUCK_CCMODE
}Buck_StateTypeDef;

typedef enum{
    BUCK_NOERROR = 0U,
    BUCK_OCP = 1U << 0,
    BUCK_OVP = 1U << 1
}Buck_FaultFlagTypeDef;

typedef struct Buck_HandleTypeDef Buck_HandleTypeDef;

#define BUCK_OCTHRESHOLD 0.5f
#define BUCK_OVTHRESHOLD 10.0f

#define BUCK_MAX_DUTY 0.90f
#define BUCK_MIN_DUTY 0.10f

Buck_HandleTypeDef* Buck_Init(Buck_InitTypeDef* init);
void Buck_DeInit(Buck_HandleTypeDef* handle);

void Buck_Start(Buck_HandleTypeDef* handle);
void Buck_Stop(Buck_HandleTypeDef* handle);
Buck_StateTypeDef Buck_GetState(Buck_HandleTypeDef* handle);
Buck_FaultFlagTypeDef Buck_GetFaultFlag(Buck_HandleTypeDef* handle);
void Buck_ClearFaultFlag(Buck_HandleTypeDef* handle);

void Buck_SetValue(Buck_HandleTypeDef* handle, float Voltage, float Current);
float Buck_GetDuty(Buck_HandleTypeDef* handle);
void Buck_Sync(Buck_HandleTypeDef* handle);


#endif /* __BUCK_H */
