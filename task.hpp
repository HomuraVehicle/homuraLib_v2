#ifndef HMR_TASK_INC
#define HMR_TASK_INC 100
#
/*=== task ===
タスク管理用関数セット
・タスクを時間とともに登録すると、指定時間間隔でタスクが駆動する。
・タスクホスト自体は、タイマー等でoperator()を定期的に呼び出して実行する。
	・登録されたタスクは、タスクシステムの呼び出しタイミングで実行の有無を判定される。
・各タスクは、実行、待機、停止の3つの状態に分類される。
	・実行：タスクバッファに登録され、指定時間ごとに実行している状態
	・待機：タスクバッファに登録されているが、実行されない状態
	・停止；タスクバッファに登録されていない状態
	・実行/待機<=>停止の遷移は、バッファ処理を伴うため低速　しばらく利用しない場合に利用
	・実行<=>待機の遷移は、時間指定変更のみのため高速　一時的に停止したい時に利用
・タスクの実行の有無等の制御は、タスク内部から、タスク外部からの二通りの方法で制御する。
	・タスク外部からの処理
		・start/quick_start/restartでタスクを開始/待機を指令
		・startでタスクを開始する。すでに登録済みのタスクに対して実行した場合、時間間隔を変更する。
			・指定時間間隔が0の場合は、待機状態になる。
			・指定時間間隔が負の場合は、そのタスクを停止する。
		・quick_startはstartより処理が高速。ただし、すでにそのタスクが登録済みでない場合しか使えない。
			・登録済みタスクに対して使用した場合は、未定義の動作となる。禁止。
		・restartでタスクの時間間隔を変更する。そのタスクが登録済みでない場合はエラーを返す。
			・startとの違いは、新規登録ができないことだけ。
		・stopでタスクを停止する。そのタスクが登録済みでない場合はエラーを返す。
			・restartで指定時間間隔を-1にした場合と同処理を行う。
	・タスク内部からの処理
		・登録関数戻り値が正の値なら、次回も実行状態
		・登録関数戻り値が0なら、待機状態に遷移
		・登録関数戻り値が負の値なら、停止

=== homuraLib_v2::task ===
v1_00/140712 hmIto
	cpp化
	task名前空間を新たに設置し、内部の定義ファイルを分離
===hmrTask===
v2_00/131207 hmIto
	破壊的変更
	個々のタスクがcounterを管理するように変更
v1_06/130921 hmIto
	task_informStart/Stopの登録用関数を追加
v1_05/130905 hmIto
	タスクをバッファから消さずに一時停止にする「待機」を追加
	状態遷移の手段を、タスク内部とタスク外部で整理
		タスク内部からは、タスクの戻り値でタスクの実行間隔/待機/停止を実行可能
		タスク外部からは、start/quick_start/restart/stop関数で制御
v1_04/130720 hmIto
	timer5に依存しない形に変更
		task_interrupt_functionを割り込み関数内で実行してもらう
v1_03/121006 hmIto
	タスク関数内でtask_stop関数の使用を禁止
	タスク関数の戻り値が真のときに、Taskを停止する機能を追加
	Taskをtaskに変更
v1_02/121005 hmIto
	タスクの割り込み処理関係の関数群を、timer5_XXX系関数でとりあえず実装
		timer5_XXX系関数群は、hmrDeviceおよびhmrDeviceConfigに依存
v1_01/120922 hmIto
	インターフェイス部分実装完了
		//タスク初期化関数
		void task_initialize(void);
		//タスク終端化関数
		void task_finalize(void);
		//タスクに登録できる確認する
		task_bool_t task_can_start(void);
		//タスクを登録
		task_bool_t task_start(task_vFp_v TaskFp);
		//タスクを登録(二重登録は回避)
		task_bool_t task_check_start(task_vFp_v TaskFp);
		//タスクを開放
		task_bool_t task_stop(task_vFp_v TaskFp);
	120922Test_hmrTask.h/cで動作確認済み
	タスクの割り込み関係を扱う以下の関数が未実装
		//タスクで使っている割り込みタイマーを有効にする
		void _task_enable_timer(void);
		//タスクで使っている割り込みタイマーを無効にする
		void _task_disable_timer(void);
		//タスククリティカル処理時に、task_start,task_check_start,task_stopを実行する可能性がある割り込みをロック
		void _task_lock_interrupt(void);
		//_task_lock_interruptでロックし割り込みを解除
		void _task_unlock_interrupt(void);
		//timer5割り込み関数(timer4と連動しているらしい
		void __attribute__((interrupt, no_auto_psv)) _T5Interrupt(void);
	timer4/timer5の設定方法なども要チェック
v1_00/120921 hmIto
	タスクの基本構造だけ設計
*/
#include"task/client_interface.hpp"
#include"task/host_interface.hpp"
#include"task/functional_host.hpp"
#include"task/utility.hpp"
#include"task/watchdog.hpp"
#
#endif
