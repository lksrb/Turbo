using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Emit;
using Turbo;

namespace Mystery
{
	public abstract class PlayerLayer
	{
		public class PlayerLayerSystem
		{
			public PlayerLayerSystem(Player player, int capacity)
			{
				m_Player = player;
				m_Layers = new Dictionary<Type, PlayerLayer>(capacity);
				m_ListenerMap = new Dictionary<Type, List<PlayerLayer>>(capacity);
			}

			public void OnUpdate()
			{
				foreach (var layer in m_Layers)
				{
					layer.Value.OnUpdate();
				}
			}

			public T PushLayer<T>() where T : PlayerLayer, new()
			{
				T layer = new T();
				layer.m_LayerSystem = this;
				layer.m_Type = typeof(T);
				layer.m_Player = m_Player;

				m_Layers.Add(typeof(T), layer);
				layer.OnAttach();

				return layer;
			}

			public struct Listener<TListener> where TListener : PlayerLayer
			{
				PlayerLayerSystem LayerSystem;

				public Listener(PlayerLayerSystem layerSystem)
				{ 
					LayerSystem = layerSystem;
				}

				public void To<TEmitter>() where TEmitter : PlayerLayer
				{
					var listenerLayer = LayerSystem.Get<TListener>();
					if (listenerLayer == null)
					{
						Log.Error("Emitter layer was null!");
						return;
					}

					if (LayerSystem.m_ListenerMap.ContainsKey(typeof(TEmitter)))
					{
						var listeners = LayerSystem.m_ListenerMap[typeof(TEmitter)];

						if(!listeners.Contains(listenerLayer))
						{
							listeners.Add(listenerLayer);
						} else
						{
							Log.Info("Listener is already listening to emitter!");
						}

					}
					else
					{
						var list = new List<PlayerLayer> { listenerLayer };
						LayerSystem.m_ListenerMap.Add(typeof(TEmitter), list);
					}
				}
			}

			public Listener<TListener> Listen<TListener>() where TListener : PlayerLayer
			{
				return new Listener<TListener>(this);
			}

			public void Emit(PlayerLayer layer, PlayerEvent playerEvent)
			{
				if (m_ListenerMap.TryGetValue(layer.m_Type, out List<PlayerLayer> listeners))
				{
					foreach (PlayerLayer listener in listeners)
					{
						listener.OnEvent(playerEvent);
					}
				}
			}

			public T Get<T>() where T : PlayerLayer
			{
				bool success = m_Layers.TryGetValue(typeof(T), out PlayerLayer layer);
				return success ? layer as T : null;
			}


			private Player m_Player;
			private Dictionary<Type, List<PlayerLayer>> m_ListenerMap; // Key - emitter type, Value - list of listeners
			private Dictionary<Type, PlayerLayer> m_Layers;
		}

		protected virtual void OnAttach() { }
		//protected virtual void OnDetach() { }
		protected virtual void OnUpdate() { }
		protected virtual void OnEvent(PlayerEvent playerEvent) { }

		protected void Emit(PlayerEvent playerEvent) => m_LayerSystem.Emit(this, playerEvent);

		protected T Get<T>() where T : PlayerLayer
		{
			return m_LayerSystem.Get<T>();
		}

		protected Player m_Player;
		private Type m_Type;
		private PlayerLayerSystem m_LayerSystem;
	}
}
