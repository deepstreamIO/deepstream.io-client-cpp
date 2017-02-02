/*
 * Copyright 2017 deepstreamHub GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef DEEPSTREAM_EVENT_HPP
#define DEEPSTREAM_EVENT_HPP

#include <functional>
#include <map>
#include <memory>
#include <string>


namespace deepstream
{
	struct Buffer;
	struct Client;
	struct Message;


	struct Event
	{
		typedef std::string Name;

		typedef std::function<void(const Buffer&)> SubscribeFn;
		typedef std::shared_ptr<SubscribeFn> SubscribeFnRef;
		typedef std::multimap<Name, SubscribeFnRef> SubscriberMap;

		typedef std::function<void(const Name&, const Buffer&)> ListenFn;
		typedef std::shared_ptr<ListenFn> ListenFnRef;
		typedef std::map<Name, ListenFnRef> ListenerMap;


		explicit Event(Client*);


		void subscribe(const Name&, const SubscribeFn&);
		void subscribe(const Name&, const SubscribeFnRef&);

		/**
		 * @return `true` if there were subscriptions with the given name,
		 * `false` otherwise.
		 */
		void unsubscribe(const Name&);
		/**
		 * @return `true` if the given subscription existed, `false` otherwise.
		 */
		void unsubscribe(const Name&, const SubscribeFnRef&);

		/**
		 * This method registers a callback that is executed whenever a client
		 * subscribes to event whose name matches the given pattern.
		 *
		 * Listeners are useful to create active data providers: processes that
		 * only send events if clients are actually interested in them
		 *
		 * There can be only one listener for a given pattern.
		 *
		 * @post `fn` is the listener for the given pattern, overwriting the
		 * previous listener if necessary.
		 *
		 * @return false if there was already a listener, true otherwise.
		 */
		void listen(const std::string& pattern, const ListenFn& fn);
		void listen(const std::string& pattern, const ListenFnRef&);
		void unlisten(const std::string& pattern);


		void notify_(const Message&);


		Client* p_client_;
		SubscriberMap subscriber_map_;
		ListenerMap listener_map_;
	};
}

#endif
