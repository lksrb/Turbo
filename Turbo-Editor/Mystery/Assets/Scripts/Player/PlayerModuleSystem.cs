using System;
using System.Collections.Generic;
using Turbo;

namespace Mystery
{
	public abstract class PlayerModule
	{
		public class PlayerModuleSystem
		{
			public PlayerModuleSystem(Player player, int capacity)
			{
				m_Player = player;
				m_Modules = new Dictionary<Type, PlayerModule>(capacity);
				m_ListenerMap = new Dictionary<Type, List<PlayerModule>>(capacity);
			}

			public void OnUpdate()
			{
				foreach (var module in m_Modules)
				{
					module.Value.OnUpdate();
				}
			}

			public T AttachModule<T>() where T : PlayerModule, new()
			{
				T module = new T();
				module.m_ModuleSystem = this;
				module.m_Type = typeof(T);
				module.m_Player = m_Player;

				m_Modules.Add(typeof(T), module);
				module.OnAttach();

				return module;
			}

			public struct Listener<TListener> where TListener : PlayerModule
			{
				PlayerModuleSystem System;

				public Listener(PlayerModuleSystem system)
				{ 
					System = system;
				}

				public void To<TEmitter>() where TEmitter : PlayerModule
				{
					var listenerModule = System.Get<TListener>();
					if (listenerModule == null)
					{
						Log.Error("Emitter layer was null!");
						return;
					}

					if (System.m_ListenerMap.ContainsKey(typeof(TEmitter)))
					{
						var listeners = System.m_ListenerMap[typeof(TEmitter)];

						if(!listeners.Contains(listenerModule))
						{
							listeners.Add(listenerModule);
						} else
						{
							Log.Info("Listener is already listening to emitter!");
						}

					}
					else
					{
						var list = new List<PlayerModule> { listenerModule };
						System.m_ListenerMap.Add(typeof(TEmitter), list);
					}
				}
			}

			public Listener<TListener> Listen<TListener>() where TListener : PlayerModule
			{
				return new Listener<TListener>(this);
			}

			public void Emit(PlayerModule layer, PlayerEvent playerEvent)
			{
				if (m_ListenerMap.TryGetValue(layer.m_Type, out List<PlayerModule> listeners))
				{
					foreach (PlayerModule listener in listeners)
					{
						listener.OnEvent(playerEvent);
					}
				}
			}

			public T Get<T>() where T : PlayerModule
			{
				bool success = m_Modules.TryGetValue(typeof(T), out PlayerModule module);
				return success ? module as T : null;
			}


			private Player m_Player;
			private Dictionary<Type, List<PlayerModule>> m_ListenerMap; // Key - emitter type, Value - list of listeners
			private Dictionary<Type, PlayerModule> m_Modules;
		}

		protected virtual void OnAttach() { }
		//protected virtual void OnDetach() { }
		protected virtual void OnUpdate() { }
		protected virtual void OnEvent(PlayerEvent playerEvent) { }

		protected void Emit(PlayerEvent playerEvent) => m_ModuleSystem.Emit(this, playerEvent);

		protected T Get<T>() where T : PlayerModule
		{
			return m_ModuleSystem.Get<T>();
		}

		protected Player m_Player;
		private Type m_Type;
		private PlayerModuleSystem m_ModuleSystem;
	}
}
