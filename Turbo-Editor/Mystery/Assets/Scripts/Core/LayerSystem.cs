using System;
using System.Collections.Generic;
using Turbo;

// TEntity and TEnum are duplicated in class scope but in this case it does not matter
#pragma warning disable CS0693
namespace Mystery
{
	// Layer
	// Layer
	// Layer

	public abstract partial class Layer<TEntity, TEnum> where TEntity : Entity
	{
		protected TEntity m_Entity;
		private Type m_Type;
		private LayerSystem<TEntity, TEnum> m_System;

		protected virtual void OnAttach() { }
		protected virtual void OnDetach() { }
		protected virtual void OnUpdate() { }
		protected virtual void OnEvent(TEnum layerEvent) { }

		protected void Emit(TEnum layerEvent) => m_System.Emit(this, layerEvent);

		protected T Get<T>() where T : Layer<TEntity, TEnum>
		{
			return m_System.Get<T>();
		}
	}

	// Layer system
	// Layer system
	// Layer system

	public abstract partial class Layer<TEntity, TEnum> where TEntity : Entity
	{
		public class LayerSystem<TEntity, TEnum> where TEntity : Entity
		{
			private readonly TEntity m_Entity;
			private Dictionary<Type, List<Layer<TEntity, TEnum>>> m_ListenerMap; // Key - emitter type, Value - list of listeners
			private Dictionary<Type, Layer<TEntity, TEnum>> m_Layers;

			public LayerSystem(TEntity entity, int capacity)
			{
				m_Entity = entity;
				m_Layers = new Dictionary<Type, Layer<TEntity, TEnum>>(capacity);
				m_ListenerMap = new Dictionary<Type, List<Layer<TEntity, TEnum>>>(capacity);
			}

			public void OnUpdate()
			{
				foreach (var layer in m_Layers)
				{
					layer.Value.OnUpdate();
				}
			}

			public T PushLayer<T>() where T : Layer<TEntity, TEnum>, new()
			{
				T layer = new T();
				layer.m_System = (Layer<TEntity, TEnum>.LayerSystem<TEntity, TEnum>)(object)this;
				layer.m_Type = typeof(T);
				layer.m_Entity = m_Entity;

				m_Layers.Add(typeof(T), layer);
				layer.OnAttach();

				return layer;
			}

			public struct Listener<TListener> where TListener : Layer<TEntity, TEnum>
			{
				private readonly LayerSystem<TEntity, TEnum> System;

				public Listener(LayerSystem<TEntity, TEnum> system)
				{
					System = system;
				}

				public void To<TEmitter>() where TEmitter : Layer<TEntity, TEnum>
				{
					var listenerLayer = System.Get<TListener>();
					if (listenerLayer == null)
					{
						Log.Error("Emitter layer was null!");
						return;
					}

					if (System.m_ListenerMap.ContainsKey(typeof(TEmitter)))
					{
						var listeners = System.m_ListenerMap[typeof(TEmitter)];

						if (!listeners.Contains(listenerLayer))
						{
							listeners.Add(listenerLayer);
						}
						else
						{
							Log.Info("Listener is already listening to emitter!");
						}
					}
					else
					{
						var list = new List<Layer<TEntity, TEnum>> { listenerLayer };
						System.m_ListenerMap.Add(typeof(TEmitter), list);
					}
				}
			}

			public Listener<TListener> Listen<TListener>() where TListener : Layer<TEntity, TEnum>
			{
				return new Listener<TListener>(this);
			}

			public void Emit(Layer<TEntity, TEnum> layer, TEnum layerEvent)
			{
				if (m_ListenerMap.TryGetValue(layer.m_Type, out List<Layer<TEntity, TEnum>> listeners))
				{
					foreach (var listener in listeners)
					{
						listener.OnEvent(layerEvent);
					}
				}
			}

			public T Get<T>() where T : Layer<TEntity, TEnum>
			{
				bool success = m_Layers.TryGetValue(typeof(T), out Layer<TEntity, TEnum> layer);
				return success ? layer as T : null;
			}
		}
	}
}
#pragma warning restore CS0693
