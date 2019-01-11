/***
 * RemoteAction provides a way to send actions to a remote component.
 * The sender creates a fresh RemoteAction and passes it down.
 * The recepient component calls subscribe.
 * The caller can then send actions to the recipient components via act.
 * This mechanism is useful in places where you'd normally use methods
 * on a ref.
 */
type t('action);

/*** Create a new remote action, to which one component will subscribe. */
let create: unit => t('action);

/*** Subscribe to the remote action, via the component's `act` function. */
let subscribe: (~send: 'action => unit, t('action)) => unit;

/*** Perform an action on the subscribed component. */
let send: (t('action), ~action: 'action) => unit;
