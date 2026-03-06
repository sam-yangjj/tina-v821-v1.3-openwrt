/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdint.h>
#include "pid.h"
#include "platform.h"

PID pid;

void InitPID(void)
{
	pid.SetTemperature = 125;
	pid.SumError = 0;
	pid.Kp = 700;

	pid.Ki = 0.600;
	pid.Kd = 0.00;

    pid.NowError = 0;
    pid.LastError = 0;

    pid.Out0 = 0;
    pid.Out = 0;
}

int LocPIDCalc(void)
{
	static int out1, out2, out3;

	pid.NowError =pid.ActualTemperature-pid.SetTemperature;
	pid.SumError += pid.NowError;

	out1 = pid.Kp * pid.NowError;
	out2 = pid.Kp * pid.Ki * pid.SumError;
	out3 = pid.Kp * pid.Kd * (pid.NowError - pid.LastError);

	pid.LastError = pid.NowError;

	return out1 + out2 + out3 + pid.Out;
}

void SetPID_Kp(int value)
{
	pid.Kp = (float)value/1000;
}

void SetPID_Ki(int value)
{
	pid.Ki = (float)value/1000;
}

void SetPID_Kd(int value)
{
	pid.Kd = (float)value/1000;
}

void SetPID_Temperature(int value)
{
	pid.SetTemperature = value;
}

int32_t pwm_r2p0(uint32_t cycle)
{
	return 0;
}

void pid_test(uint32_t T)
{
	uint32_t Temperature;
	int temp=0,PWMvalue=0;
	InitPID();



	Temperature=thermal_temperature_get();
	if(Temperature>T)
	{
		pid.SetTemperature =T;
		pid.ActualTemperature = Temperature;
		temp = LocPIDCalc();
		if(temp >3072)
			PWMvalue = 3072;
		else if(temp < 0)
			PWMvalue = 0;
		else
			PWMvalue = (unsigned int)temp;
	}
	else
		PWMvalue = 0;

	pwm_r2p0(PWMvalue);

}
