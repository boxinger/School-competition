#include "PFC.h"

#include <stdlib.h>

#ifdef ARM_MATH_DSP
    #include "arm_math.h"
#else
    #include <math.h>
#endif

struct PFC_HandleTypeDef {
    PFC_InitTypeDef Init;
    PID_HandleTypeDef* OutPutVoltagePID;
    PID_HandleTypeDef* InductorCurrentPID;
    PFC_StateTypeDef State;
    PFC_FaultCodeTypeDef FaultCode;
    float TargetOutputVoltage;
    float Duty;
    float OutputVoltageFiltered;
    float G;
    PFC_PolarityTypeDef Polarity;
    uint16_t VoltageLoopCounter;
    uint16_t CurrentLoopCounter;
};

PFC_HandleTypeDef*  PFC_Init(PFC_InitTypeDef* init) {
    // Verify input parameters
    if (init == NULL 
        || init->InitFunc == NULL
        || init->DeInitFunc == NULL
        || init->Start == NULL
        || init->Stop == NULL
        || init->SetDuty == NULL
        || init->GetInputVoltage == NULL
        || init->GetOutputVoltage == NULL
        || init->GetInductorCurrent == NULL) {
        return NULL;
    }

    // Allocate memory for PFC handle
    PFC_HandleTypeDef* handle = malloc(sizeof(PFC_HandleTypeDef));
    if (handle == NULL) {
        return NULL;
    }

    // Load initialization parameters
    (*handle) = (PFC_HandleTypeDef){0}; 
    handle->Init = *init;
    handle->TargetOutputVoltage = 0.0f;
    handle->Duty = 0.0f;
    handle->State = PFC_DISABLED;
    handle->FaultCode = PFC_NOERROR;
    handle->OutputVoltageFiltered = 0.0f;
    handle->G = 0.0f;

    // Call user-defined initialization function
    init->InitFunc();

    //PID initialization
    handle->OutPutVoltagePID = PID_Init(init->OutPutVoltagePIDInit);
    if (handle->OutPutVoltagePID == NULL) {
        PFC_DeInit(handle);
        return NULL;
    }
    handle->InductorCurrentPID = PID_Init(init->InductorCurrentPIDInit);
    if (handle->InductorCurrentPID == NULL) {
        PFC_DeInit(handle);
        return NULL;
    }

    return handle;
}


void PFC_DeInit(PFC_HandleTypeDef* handle) {
    // Verify input parameter
    if (handle == NULL) {
        return;
    }

    handle->Init.Stop();

    // Call user-defined de-initialization function
    handle->Init.DeInitFunc();

    // Free PID handles
    if (handle->OutPutVoltagePID != NULL) {
        PID_DeInit(handle->OutPutVoltagePID);
    }
    if (handle->InductorCurrentPID != NULL) {
        PID_DeInit(handle->InductorCurrentPID);
    }

    // Free PFC handle memory
    free(handle);
}


void PFC_Start(PFC_HandleTypeDef* handle) {
    if (handle == NULL) {
        return;
    }
    if (handle->State == PFC_ERROR
        || handle->State == PFC_ENABLED) {
        return; // Already started
    }
    PID_Reset(handle->OutPutVoltagePID);
    PID_Reset(handle->InductorCurrentPID);
    handle->State = PFC_ENABLED;
    handle->Init.Start();
}


void PFC_Stop(PFC_HandleTypeDef* handle) {
    if (handle == NULL) {
        return;
    }
    handle->Init.Stop();
    handle->State = PFC_DISABLED;
}


PFC_StateTypeDef PFC_GetState(PFC_HandleTypeDef* handle) {
    if (handle == NULL) {
        return PFC_ERROR;
    }
    return handle->State;
}


PFC_FaultCodeTypeDef PFC_GetFaultCode(PFC_HandleTypeDef* handle) {
    if (handle == NULL) {
        return PFC_INVALID;
    }
    return handle->FaultCode;
}


void PFC_ClearFaultCode(PFC_HandleTypeDef* handle) {
    if (handle == NULL) {
        return;
    }
    if (handle->State != PFC_ERROR
        && handle->State != PFC_DISABLED) {
        return; // Only clear faults if currently in error state
    }
    handle->FaultCode = PFC_NOERROR;
    handle->State = PFC_DISABLED;
}

void PFC_SetOutputVoltage(PFC_HandleTypeDef* handle, float Voltage) {
    if (handle == NULL) {
        return;
    }
    handle->TargetOutputVoltage = Voltage;
}

void PFC_Sync(PFC_HandleTypeDef* handle) {
    if (handle == NULL) {
        return;
    }
    if (handle->State == PFC_ERROR) {
        return; // Do not operate if in error state
    } else if (handle->State == PFC_DISABLED) {
        return; // Do not operate if disabled
    }

    float outputVoltage = handle->Init.GetOutputVoltage();
    float inputVoltage = handle->Init.GetInputVoltage();
    float inductorCurrent = handle->Init.GetInductorCurrent();

    // Protection
    if (outputVoltage > PFC_OVTHRESHOLD){
        PFC_Stop(handle);
        handle->State = PFC_ERROR;
        handle->FaultCode = PFC_OVP;
        return;
    }
    if (inductorCurrent > PFC_OCTHRESHOLD
        || inductorCurrent < -PFC_OCTHRESHOLD){
        PFC_Stop(handle);
        handle->State = PFC_ERROR;
        handle->FaultCode = PFC_OCP;
        return;
    }

    if (inputVoltage > PFC_ZeroCrossingThreshold){
        handle->Polarity = PFC_Positive;
    } else if (inputVoltage < -PFC_ZeroCrossingThreshold){
        handle->Polarity = PFC_Negative;
    } else {
        handle->Polarity = PFC_ZeroCrossing;
    }

    handle->VoltageLoopCounter++;
    handle->CurrentLoopCounter++;

    if (handle->Polarity == PFC_ZeroCrossing) {
        handle ->Stop();
    } else {
        handle ->Start();
    }

    if (handle ->VoltageLoopCounter >= PFC_VoltageLoopMaxCounter) { 
        handle->VoltageLoopCounter = 0;

        float alpha = 0.03f; //f_c = 1kHz * alpha/2¦Ð ¡Ö 4.8Hz
        handle->OutputVoltageFiltered = alpha * outputVoltage + (1 - alpha) * handle->OutputVoltageFiltered;
        float outputVoltageError = handle->TargetOutputVoltage - handle->OutputVoltageFiltered;
        handle->G = PID_ComputeConditional(handle->OutPutVoltagePID, handle->TargetOutputVoltage, handle->OutputVoltageFiltered, 0.0f, 0.5f);
    }
    
    if (handle->CurrentLoopCounter >= PFC_CurrentLoopMaxCounter) { 
        handle->CurrentLoopCounter = 0;
        float i_ref = handle->G * inputVoltage;
        float inductorCurrentError = i_ref - inductorCurrent;
        float duty = PID_ComputeConditional(handle->InductorCurrentPID, i_ref, inductorCurrent, 0.0f, 1.0f);
        handle->Duty = duty;
    }
    
    handle->Init.SetDuty(handle->Duty);

}
