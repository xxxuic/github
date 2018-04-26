// uic 2017.10.20
// очистил CheckBox от скверны ресурсной зависимости

#pragma once

#include "BmpButton.h"

// универальный класс для смены значений переменной в каком-то наборе значений
template <typename T> struct StateButton : BmpButton
{
	StateButton<T>()
	{
	}
	~StateButton<T>()
	{
		if (m_state_values)
			delete[] m_state_values;
	}
	void OnCreate()
	{
		SetMessageHandler(WM_LBUTTONUP, (InterfaceObject_OwnFunction)(&StateButton<T>::OnMouseUp));
	}
	void OnMouseUp()
	{
		if (m_pressed && m_enabled)
		{
			m_current_state++;
			if (m_current_state >= m_states_num)
				m_current_state = 0;
			*m_value = m_state_values[m_current_state];
			OnUpdateValue();
		}
		BmpButton::OnMouseUp();
	}
	void Setup(T* value, int num_states, T* state_values)
	{
		m_value = value;
		m_states_num = num_states;
		if (m_state_values)
			delete[] m_state_values;
		m_state_values = new T[m_states_num];
		memcpy(m_state_values, state_values, sizeof(T) * m_states_num);
		UpdateValue();
	}
	virtual void OnUpdateValue() {}
	void UpdateValue()
	{
		for (m_current_state = 0; m_current_state < m_states_num; m_current_state++)
			if (m_state_values[m_current_state] == *m_value)
				break;
		if (m_current_state == m_states_num)
		{
			m_current_state = 0;
			*m_value = m_state_values[m_current_state];
		}
		OnUpdateValue();
		Invalidate();
	}
	int m_states_num, m_current_state;
	T *m_value, *m_state_values;
};

// Кнопка для выбора выбора значений булевой переменной
struct CB : StateButton<bool>
{
	CB();

	virtual void OnCreate();

	virtual void OnUpdateValue();

	void Setup(bool* value, CString name);

	virtual void OnMouseOut();
	virtual void OnMouseIn();

	static int m_box_size;
protected:
	DWORD m_tmp_norm_color;
	virtual void OnChangeSize();
	virtual void Draw(CDC* dc);
};

////////////////////////////////////////////////////////////////////////////////
// end