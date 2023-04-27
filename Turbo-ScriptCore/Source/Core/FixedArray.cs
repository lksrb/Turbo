namespace Turbo
{
	public struct FixedArray<T>
	{
		private T[] m_Data;
		private uint m_Index;

		public FixedArray(uint size = 3)
		{
			m_Data = new T[size];
			m_Index = 0;
		}

		public void PushBack(T value)
		{
			if (m_Index >= m_Data.Length)
			{
				Log.Error("Overflowing the buffer!");
				return;
			}

			m_Data[m_Index] = value;
			m_Index++;
		}

		public T this[uint index]
		{
			get
			{
				if (index >= m_Data.Length)
				{
					Log.Error("Indexing outside of the buffer!");
					return default;
				}
				return m_Data[index];
			}
			set
			{
				if (index >= m_Data.Length)
				{
					Log.Error("Indexing outside of the buffer!");
					return;
				}

				m_Data[index] = value;
			}
		}

		public uint Length => m_Index;
	}
}
