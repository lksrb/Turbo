namespace Turbo
{
	public struct FixedArray<T>
	{
		private T[] m_Data;
		private uint m_Size;
		private uint m_Capacity;

		public FixedArray(uint capacity = 3)
		{
			m_Data = new T[capacity];
			m_Capacity = capacity;
			m_Size = 0;
		}

		public void PushBack(T value)
		{
			if (m_Size >= m_Capacity)
			{
				Log.Error("Overflowing the buffer!");
				return;
			}

			m_Data[m_Size] = value;
			m_Size++;
		}

		public T this[uint index]
		{
			get
			{
				if (index >= m_Capacity)
				{
					Log.Error("Indexing outside of the buffer!");
					return default;
				}
				return m_Data[index];
			}
			set
			{
				if (index >= m_Capacity)
				{
					Log.Error("Indexing outside of the buffer!");
					return;
				}

				m_Data[index] = value;
			}
		}

		public uint Length => m_Size;
	}
}
