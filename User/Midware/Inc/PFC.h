#ifndef __PFC_H
#define __PFC_H

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
} PFC_InitTypeDef;

typedef struct PFC_HandleTypeDef PFC_HandleTypeDef;

typedef enum{
    PFC_ERROR = 0U,
    PFC_DISABLED,
    PFC_ENABLED
}PFC_StateTypeDef;

typedef enum{
    PFC_INVALID = 0U,
    PFC_NOERROR,
    PFC_OCP,
    PFC_OVP
}PFC_FaultCodeTypeDef;

#define PFC_OVTHRESHOLD 60.0f
#define PFC_OCTHRESHOLD 2.0f

typedef enum{
    PFC_Positive = 0U,
    PFC_Negative,
    PFC_ZeroCrossing
}PFC_PolarityTypeDef;

#define PFC_ZeroCrossingThreshold 1.0f

#define PFC_VoltageLoopMaxCounter 100U
#define PFC_CurrentLoopMaxCounter 10U

PFC_HandleTypeDef* PFC_Init(PFC_InitTypeDef* init);
void PFC_DeInit(PFC_HandleTypeDef* handle);

void PFC_Start(PFC_HandleTypeDef* handle);
void PFC_Stop(PFC_HandleTypeDef* handle);
PFC_StateTypeDef PFC_GetState(PFC_HandleTypeDef* handle);
PFC_FaultCodeTypeDef PFC_GetFaultCode(PFC_HandleTypeDef* handle);
void PFC_ClearFaultCode(PFC_HandleTypeDef* handle);

void PFC_SetOutputVoltage(PFC_HandleTypeDef* handle, float Voltage);
void PFC_Sync(PFC_HandleTypeDef* handle);

#endif /* __PFC_H */
