//Functions
extern void TIMER2_EVENT(void);
extern void START_TIMER2(void);
extern void STOP_TIMER2(void);
extern void MX_TIM2_Init(void);

extern void TIM5_ReInit(void);
extern void MX_TIM5_DeInit(void);
extern void TIMER5_CTRL(void);

//Variables
extern uint16_t timer2_val;
extern int shw_discnt_flag;

extern uint32_t uhCapture;
