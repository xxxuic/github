// uic 20.1.2010

#pragma once

template<typename T> struct ElasticBuffer
{
	ElasticBuffer<T>():
		m_buff(NULL), m_pointer(NULL), m_max_size(0), m_current_size(0), m_filled_size(0)
	{
	}

	~ElasticBuffer()
	{
		Delete();
	}

	void ChangeFilledSize(int delta)
	{
		SetFilledSize(max(0, GetFilledSize() + delta));
		if (GetFilledSize() >= GetCurrentSize())
			UpdateSize((GetFilledSize() + 10) * 2);
	}

	virtual bool AddValue(T value)
	{
		if (GetFilledSize() >= GetCurrentSize())
		{
			UpdateSize((GetFilledSize() + 10) * 2);
			//return false;
		}
		*(GetBuff() + GetFilledSize()) = value;
		SetFilledSize(GetFilledSize()+ 1);
		return true;
	}

	virtual bool AddValue(T* value)
	{
		return AddValue(*value);
	}

	virtual bool SetValue(T value, int pos)
	{
		if (pos < 0 || pos >= GetCurrentSize())
			return false;
		*(GetBuff() + pos) = value;
		return true;
	}

	virtual bool SetValue(T* value, int pos)
	{
		return SetValue(*value, pos);
	}

	void Delete()
	{
		Clear();
		if (m_buff && m_pointer == NULL)
			delete[] m_buff;
		m_buff = NULL;
		m_current_size = m_max_size = 0;
		OnBuffPtrChanged();
	}

	virtual void OnBuffPtrChanged() {};
	virtual void OnClear() {};

	void Clear()
	{
		UpdateSize(0);
		StartFill();
		OnClear();
	}

	void SetPointer(ElasticBuffer<T>* other)
	{
		Delete();
		m_pointer = other;
	}

	void UpdateSize(int new_size, bool keep_data = true)
	{
		if (m_pointer)
		{
			m_pointer->UpdateSize(new_size, keep_data);
			return;
		}
		int old_size = m_current_size;
		m_current_size = new_size;
		if (new_size <= m_max_size)
			return;
		m_max_size = new_size;
		T* tmp = m_buff;
		m_buff = new T[m_max_size];
		if (tmp != NULL)
		{
			if (keep_data)
				memcpy(m_buff, tmp, old_size * sizeof(T)); // для RingBuff будут проблемы
			else
			{
				memset(m_buff, 0, new_size * sizeof(T));
				Clear();
			}
			delete[] tmp;
		}
		OnBuffPtrChanged();
	}
	void Shrink()
	{
		if (m_pointer)
		{
			m_pointer->Shrink();
			return;
		}
		if (m_buff == NULL)
		{
			UpdateSize(m_current_size);
			return;
		}
		if (m_current_size == m_max_size)
			return;
		T* new_buff = new T[m_current_size];
		memcpy(new_buff, m_buff, m_current_size * sizeof(T));
		m_max_size = m_current_size;
		swap(new_buff, m_buff);
		delete[] new_buff;
	}
	T* GetBuff()
	{
		if (m_pointer)
			return m_pointer->GetBuff();
		return m_buff;
	}
	void SwapBuffers(ElasticBuffer<T> &other)
	{
		if (m_pointer != NULL || other.m_pointer != NULL)
			return;
		swap(m_buff, other.m_buff);
		swap(m_max_size, other.m_max_size);
		swap(m_current_size, other.m_current_size);
	}
	void StartFill() { SetFilledSize(0); }
	void FinishFill() { UpdateSize(GetFilledSize()); }
	int GetCurrentSize() { return (m_pointer) ? m_pointer->GetCurrentSize() : m_current_size; }
	int GetFilledSize() { return (m_pointer) ? m_pointer->GetFilledSize() : m_filled_size; }
	void SetFilledSize(int size) { if (m_pointer) m_pointer->SetFilledSize(size); else m_filled_size = size; }
	//int GetMaxSize() { return m_max_size; }
private:
	ElasticBuffer<T> operator = (ElasticBuffer<T>& other)
	{
		// запрещенный оператор
		// для отлова ситуаций buff1 = buff2
		// заменить на buff1.SetPointer(&buff2)
		return *this;
	}
	int m_max_size, m_current_size, m_filled_size;
	T* m_buff;
	ElasticBuffer<T>* m_pointer;
};

template <typename T> void ElasticBuffer_DeletePointerBuffer(ElasticBuffer<T*> &eb)
{
	T** ptr = eb.GetBuff();
	int i, isz = eb.GetFilledSize();
	eb.SetFilledSize(0);
	for (i = 0; i < isz; i++, ptr++)
		if (*ptr)
			delete *ptr;
	eb.Clear();
}
////////////////////////////////////////////////////////////////////////////////
// end